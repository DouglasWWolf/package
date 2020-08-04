//=================================================================================================
// package.cpp - Used to package multiple executables and support files into a single package with
//               the version number stamped in a known, standard format
//=================================================================================================

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include "cprocess.h"
#include "typedefs.h"
using std::string;

struct version_t
{
    char    name[256];
    int     major;
    int     minor;
    int     build;
};

#pragma pack(push, 1)
struct header_t
{
    uint64_t    magic;
    u32be       header_version;
    u32be       file_size;
    uint8_t     filler1[48];
    uint8_t     gw_marker[4];
    char        type[64];
    uint8_t     filler2[124];
    uint8_t     major;
    uint8_t     minor;
    uint8_t     build_h;
    uint8_t     build_l;
    uint8_t     filler3[252];
} header;
#pragma pack(pop)

// This is the magic number for our file type
#define MAGIC_NUMBER 0xDEADACDCDCACADDEULL

// This is the name of the tarball we're going to create
const char* tarball = "package.tgz";

// This will be true if we're packaging up a gateway
bool is_gateway = false;

//=================================================================================================
// parse_dotted_version() - Extracts a dotted version like "x.y.z" into individual integers
//=================================================================================================
void parse_dotted_version(char* ptr, version_t* p_result)
{
    version_t& version = *p_result;

    // Extract the major version
    version.major = atoi(ptr);

    // Extract minor version
    ptr = strchr(ptr+1, '.');
    if (ptr)
    {
        version.minor = atoi(ptr+1);
        ptr = strchr(ptr+1, '.');
    }

    // Extract the build version
    if (ptr) version.build = atoi(ptr+1);
}
//=================================================================================================


//=================================================================================================
// create_tarball() - Creates the tarball from our list of source files
//=================================================================================================
void create_tarball(char** file_list)
{
    CProcess process;

    // Build the tar command line
    char* buffer = new char[4096];

    // Build the string we'll use to create the tarball
    strcpy(buffer, "tar cvzf ");
    strcat(buffer, tarball);
    int idx = 0;
    while (file_list[idx])
    {
        // Fetch this filename
        const char* filename = file_list[idx];

        // Don't allow users to duplicate the first filename
        if (idx > 0 && strcmp(filename, file_list[0]) == 0) continue;

        // And append this filename to the tar command we're going to execute
        strcat(buffer, " ");
        strcat(buffer, filename);

        // Go point to the next filename
        ++idx;
    }

    // Tar all of our files into a tarball
    process.run(false, buffer);

    // We don't need that buffer anymore
    delete[]  buffer;
}
//=================================================================================================



//=================================================================================================
// get_version_stamp_from_file() - Fetches the version number from a binary file
//=================================================================================================
void  get_version_stamp_from_file(const char* filename, version_t* p_result)
{
    FILE*   ifile;
    char*   tag = "EXEVERSION ";
    int     tag_len = strlen(tag);
    bool    found = false;
    char*   ptr;

    // We don't have any output values yet
    memset(p_result, 0, sizeof *p_result);

    // For a name, use the filename without any extension
    strcpy(p_result->name, filename);
    ptr = strchr(p_result->name, '.');
    if (ptr) *ptr = 0;

    // Just in case this has directory names, reduce it to just the filename
    ptr = strchr(p_result->name, '/');
    while (ptr && strchr(ptr+1, '/')) ptr = strchr(ptr+1, '/');
    if (ptr) strcpy(p_result->name, ptr);

    // Open the input file
    ifile = fopen(filename, "rb");
    if (ifile == nullptr)
    {
        fprintf(stderr, "can't open file: %s\n", filename);
        exit(1);
    }

    // Find out how long it is
    fseek(ifile, 0, SEEK_END);
    auto file_size = ftell(ifile);
    rewind(ifile);

    // We're going to allocate just a bit more than the file so that we don't read
    // off the end of our buffer later
    auto buffer_size = file_size + 10;

    // Allocate enough memory to hold the entire file plus a little room on the end
    char* buffer = new char[buffer_size];

    // For safety, lets make sure it's filled with nul bytes
    memset(buffer, 0, buffer_size);

    // Now read in the entire file
    fread(buffer, 1, file_size, ifile);
    fclose(ifile);

    // Now search the buffer for our tag
    ptr = buffer;
    for (int i=0; i<file_size; ++i)
    {
        if (*ptr == *tag && strncmp(ptr, tag, tag_len) == 0)
        {
            found = true;
            break;
        }
        ++ptr;
    }

    // If no version tag was found, we can't package this
    if (!found)
    {
        printf("no version number found in %s\n", filename);
        exit(1);
    }

    // Parse the x.y.z into integers
    parse_dotted_version(ptr + tag_len, p_result);

    // We don't need the file in the buffer anymore
    delete[] buffer;
}
//=================================================================================================


//=================================================================================================
// make_header() - Fills in the header structure
//=================================================================================================
void make_header(version_t& version)
{
    // Clear the header
    memset(&header, 0, sizeof header);

    // Set the magic number that identifies this as one of ours
    header.magic = MAGIC_NUMBER;

    // This is version 1 of this file format
    header.header_version = 1;

    // Fill in the file type
    strcpy(header.type, version.name);

    // And fill in the most important item, the version number of this package
    header.major   = version.major;
    header.minor   = version.minor;
    header.build_h = version.build >> 8;
    header.build_l = version.build & 0xFF;

    // If this is a gateway package, set the marker so legacy software knows this is gateway code
    if (is_gateway) memcpy(header.gw_marker, "*UF*", 4);

}
//=================================================================================================

//=================================================================================================
// make_package() - creates a version of our tarball with our 512 byte header pre-pended
//=================================================================================================
void make_package()
{
    FILE *ifile, *ofile;
    char package[256];

    // This is the name of the package that we're going to create
    int build = (header.build_h << 8) | header.build_l;
    sprintf(package, "%s_%i.%i.%02i.img", header.type, header.major, header.minor, build);

    // Open that tarball we created
    ifile = fopen(tarball, "rb");
    if (ifile == nullptr)
    {
        fprintf(stderr, "Failed to re-open %s\n", tarball);
        exit(1);
    }

    // Find out how long it is
    fseek(ifile, 0, SEEK_END);
    auto file_size = ftell(ifile);
    rewind(ifile);

    // Write the length of the file into our header
    header.file_size = file_size;

    // Allocate enough memory to hold the entire tarball file
    char* tarball_data = new char[file_size];

    // Now read in the entire file
    fread(tarball_data, 1, file_size, ifile);
    fclose(ifile);

    ofile = fopen(package, "wb");
    if (ofile == nullptr)
    {
        fprintf(stderr, "can't create file %s\n", package);
    }

    // Write the header
    fwrite(&header, 1, sizeof header, ofile);

    // Write the tarball data
    fwrite(tarball_data, 1, file_size, ofile);

    // We've written out package!
    fclose(ofile);

    // We don't need that giant buffer anymore
    delete[] tarball_data;

    printf("Made package %s\n", package);
}
//=================================================================================================


//=================================================================================================
// main() - This program creates a tarball and puts a 512 byte header on it that identifies the
//          version number of the main file that it contains
//=================================================================================================
int main(int argc, char** argv)
{
    version_t   version;
    CProcess    process;

    // Make sure we have a filename on the command line
    if (argv[1] == nullptr || argv[2] == nullptr)
    {
        fprintf(stderr, "Missing parameter on command line\n");
        exit(1);
    }

    // Find out whether this is gateway or firmware
    if (strcmp(argv[1], "gw") == 0)
        is_gateway = true;
    else if (strcmp(argv[1], "fw") == 0)
        is_gateway = false;
    else
    {
        fprintf(stderr, "type must be either gw or fw\n");
        exit(1);
    }

    // Point to the first filename
    argv = &argv[2];

    // Keep track of the filename of the first file
    const char* first_filename = argv[0];

    // Fetch the version information from the file
    get_version_stamp_from_file(first_filename, &version);

    // Create the header
    make_header(version);

    // Create the tarball
    create_tarball(&argv[0]);

    // Create the package
    make_package();

    // And get rid of the tarball file that we don't need anymore
    remove(tarball);
}
//=================================================================================================

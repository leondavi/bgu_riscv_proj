/* NIST Secure Hash Algorithm */

#include "sha.h"

int main(int argc, char **argv)
{
    SHA_INFO sha_info;
    sha_stream(&sha_info);
    return(0);
}

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include "downloader.cpp"

struct GlobalArgs {
  std::string out_file_name; //-o option
  bool verbose;              //-v option
  std::string url;
} global_args;

static const char *opt_string = "o:vh?";

void display_usage() {
  std::cout << "usage: -o output filename -v verbose log" << std::endl;
}

int main(int argc, char **argv) {
  /* Initialize globalArgs before we get to work. */
  global_args.verbose = false;

  int opt = 0;

  while ((opt = getopt(argc, argv, opt_string)) != -1) {
    switch (opt) {
    case 'o':
      global_args.out_file_name = optarg;
      break;
    case 'v':
      global_args.verbose = true;
      break;
    case 'h':
    case '?':
      display_usage();
      break;
    default:
      printf("optopt = %c\n", (char)optopt);
      printf("opterr = %d\n", opterr);
      display_usage();
      exit(EXIT_FAILURE);
    }
  }

  global_args.url = argv[optind];

  auto downloader = std::make_shared<fftool::Downloader>(
      global_args.url, global_args.out_file_name);

  downloader->SyncStart();

  return 0;
}

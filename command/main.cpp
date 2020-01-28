#include <cstdlib>
#include <ffnetwork/client.h>
#include <iostream>
#include <unistd.h>

struct GlobalArgs {
  const char *out_file_name; //-o option
  bool verbose;              //-v option
  const char *url;
} global_args;

static const char *opt_string = "o:vh?";

void display_usage() {
  std::cout << "usage: -o output filename -v verbose log" << std::endl;
}

int main(int argc, char **argv) {
  /* Initialize globalArgs before we get to work. */
  global_args.out_file_name = NULL;
  global_args.verbose = false;
  global_args.url = NULL;

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

  auto client = ffnetwork::CreateClient();
  auto request = ffnetwork::CreateRequest(global_args.url, {});
  auto response = client->PerformRequestSync(request);

  size_t data_length = 0;
  std::cout << response->data(data_length);

  return 0;
}

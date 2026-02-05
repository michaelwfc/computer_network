#include "socket.hh"
#include "util.hh"

#include <cstdlib>
#include <iostream>

using namespace std;

/**
## Idea/Solution for the get_URL() function
### Key Points:

1. **Create a TCP socket**: Use the `TCPSocket` class from the Sponge library
2. **Connect to the server**: Use `Address(host, "http")` to connect to port 80
3. **Construct HTTP request**: Follow the format you used with telnet:
   - GET request line with the path
   - Host header with the host name
   - Connection: close header to signal end of requests
   - Empty line to terminate headers
4. **Send the request**: Use `sock.write()` to send the request
5. **Read the response**: Loop until `sock.eof()` is true, reading and printing
the response
6. **Use `\r\n`**: HTTP protocol requires `\r\n` line endings, not just `\n`
7. **Read all data**: Keep reading until EOF to get the complete response

### Why This Approach Works:

- **RAII**: The socket will be automatically closed when it goes out of scope
- **Connection: close**: Tells the server to close the connection after
responding
- **EOF detection**: When the server closes the connection, `sock.eof()` will
become true
- **Complete reading**: Continuously reading until EOF ensures you get the full
response

This implementation follows the same pattern you used manually with telnet but
automates the process using the Sponge library's TCP socket wrapper.
*/
void get_URL(const string &host, const string &path) {
  // Your code here.

  // You will need to connect to the "http" service on
  // the computer whose name is in the "host" string,
  // then request the URL path given in the "path" string.

  // Then you'll need to print out everything the server sends back,
  // (not just one call to read() -- everything) until you reach
  // the "eof" (end of file).

  // cerr << "Function called: get_URL(" << host << ", " << path << ").\n";
  // cerr << "Warning: get_URL() has not been implemented yet.\n";

  TCPSocket sock;
  sock.connect(Address(host, "http"));
  string request = "GET " + path + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n";
  sock.write(request);

  while (!sock.eof()) {
    cout << sock.read();
  }
  sock.close();
}

int main(int argc, char *argv[]) {
  try {
    if (argc <= 0) {
      abort(); // For sticklers: don't try to access argv[0] if argc <= 0.
    }

    // The program takes two command-line arguments: the hostname and "path"
    // part of the URL. Print the usage message unless there are these two
    // arguments (plus the program name itself, so arg count = 3 in total).
    if (argc != 3) {
      cerr << "Usage: " << argv[0] << " HOST PATH\n";
      cerr << "\tExample: " << argv[0] << " stanford.edu /class/cs144\n";
      return EXIT_FAILURE;
    }

    // Get the command-line arguments.
    const string host = argv[1];
    const string path = argv[2];

    // Call the student-written function.
    get_URL(host, path);
  } catch (const exception &e) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

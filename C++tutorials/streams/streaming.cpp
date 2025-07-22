/**
 * 1. Streams are a general interface to read and write data in programs： a general input/output(IO) abstraction for C++
 * 2. Input and output streams on the same source/destination type compliment each other!
 * 3. Don’t use getline() and std::cin() together, unless you really really have to!
 * 
 */

#include <iostream>
#include <sstream>
#include <fstream>

// using namespace std;

/**
 * @brief outputStream
 * 
 * std::cout writing out something to the console, inherited from std::ostream,  has the property of the std::basic_ostream type
 *   - std  the standard namespace — where all the standard C++ library functions, classes, and objects live.
     - ::   The scope resolution operator, used to access something inside a namespace or class.
     - count Stands for "character output" — it's an object of type std::ostream used to output data to the screen.
     - <<  the insertion operator,send to the output stream
     - std::endl	Moves to a new line (like \n) and flushes the buffer.
 * 
 * std::string:  C++ string vs C string: char[]
 * std::stringstream: not connected to any external source
 */
void outputStream(){
    
    std::string hello = "Hello World";

    std::cout << hello << std::endl;

    // create a stringstream

    std::string initial_quote = "Bjarne Stroustrup C makes it easy to shoot yourself in the foot";
    std::stringstream ss(initial_quote); //initialize stringstream with string constructor
    // std::stringstream ss; // since this is a stream we can also insert the initial_quote like this!
    // ss << initial_quote;

    std::string first;
    std::string last;
    std::string language, extracted_quote;
    
    // ss >> first >> last >> language>> extracted_quote; // The >> operator only reads until the next whitespace!

    ss >> first >> last >> language;
    // getline() reads an input stream, is, up until the delim char and stores it in some buffer, str.
    std::getline(ss, extracted_quote);
    std::cout << first << " " << last << " said this: " << language << " " << extracted_quote << std::endl;


    // std::cout stream is line-buffered
    double tao =6.28;
    std::cout << tao;
    // contents in buffer not shown on external source until an explicit flush occurs!
    // flushing is an expensive operation!
    // std::cout << std::flush;
    // std::endl tells the cout stream to end the line! 
    // std::endl also tells the stream to flush
    std::cout << std::endl;

    for(int i=1;i<=5;i++){
        // std::cout << i<< std::endl;
        std::cout <<i; // output: "12345"
    }

}

/**
 * @brief input Stream with std::istream
 * 
 * a way to read data from an destination/external source
 *  - use the >> extractor operator to read from the input stream 
 *  - std::cin is the console input stream, reading in something from the console,Are inherited from std::istream
 * 
 * std::cin is buffered, leaves the newline in the buffer
 * getline() - gets rid of the newline
 * 
 * std::cin buffer stops at a whitespace
 * Whitespace in C++ includes:
 *   - " " – a literal space
 *   - \n character
 *   - \t character
 */
void inputStream(){
    std::cout << "Verifying PI..." << std::endl;
    double pi;
    std::cin;  // cin buffer is empty so prompts for input!
    std::cin >> pi; //cin not empty so it reads up to white space and saves it to double pi
    std::cout << "PI is " << pi << std::endl;

    double tao;
    std::string name;
    
    std::cin>>pi; // cin prompts user to enter a value saved in pi
    
    // std::cin>>name; // cin prompts user to enter a value saved in name
    // Notice that cin only reads until the next whitespace
    std:: getline(std::cin, name);
    std:: getline(std::cin, name);

    std::cin >>tao; // cin prompts user to enter a value saved in
    std::cout <<"My name is "<<name<<" and PI is "<<pi<<" and TAO is "<<tao<<'\n';
}

/**
 * @brief Output stream to a file
 */
void outputFileStream(){
    // Creates an output file stream to the file “hello.txt”
    std::ofstream ofs("hello.txt");
    if(ofs.is_open()){
        ofs << "Hello World!CS106L" << '\n';
    }
    ofs.close();
    ofs<<"this will not get written to the file";
    ofs.open("hello.txt",std::ios::app); // Flag specifies you want to append, not truncate!
    ofs<<"this will get written to the file!It is open again";
    }

void inputFileStream(){ 
    std::ifstream ifs("input.txt");
    if(ifs.is_open()){
        std::string line;
        std::getline(ifs, line);
        std::cout << "Read from the file: " << line << '\n';
        // while(std::getline(ifs,line)){
        //     std::cout<<line<<'\n';
        }
    if(ifs.is_open()){
        std::string lineTwo;
        std::getline(ifs,lineTwo);
        std::cout<<"Read from the file: "<<lineTwo<<'\n';
    }



}
int main(){
    // outputStream();
    // outputFileStream();
    // inputFileStream();

    inputStream();

    return 0;
}
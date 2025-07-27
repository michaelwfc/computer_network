#include <iostream>

/**
 * target_link_libraries 使得可以像使用系统和第三方库一样
 * 使用子目录中提供的公开头文件。
 *
 *  * 由于 libanswer 链接 libcurl 时使用的是 PRIVATE，
 * 在这里我们无法 #include <curl/curl.hpp>。
 *
 * 也就是说使用 PRIVATE 声明的依赖，只在构建 libanswer
 * 时可见，不会溢出并影响 libanswer 的使用者。
 *
 */

#include <answer/answer.hpp>

int main(int argc, char *argv[])
{
    // int expected_answer = answer::find_the_ultimate_answer();
    for (;;)
    {
        std::cout << "What is the ultimate answer? " << std::endl;
        // int answer;
        std::string answer;
        std::cin >> answer;
        auto expected_answer = answer::find_the_ultimate_answer();
        std::cout << "The expected answer from wolframalpha is " << std::endl
                  << expected_answer << std::endl;
        // if (answer == expected_answer)
        if (answer::check_the_answer(answer, expected_answer))
        {
            {
                std::cout << "Correct!" << std::endl;
                break;
            }
        }
        return 0;
    }
}

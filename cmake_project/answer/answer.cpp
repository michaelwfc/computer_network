// #include <iostream>
// // #include "answer/answer.hpp"
// // #include <curl/curl.h>

// #include <wolfram/alpha.hpp>


// namespace answer
// {
//   namespace v1
//   {
//     int find_the_ultimate_answer(){
//       return 42;
//     }
//   }

//   namespace v2
//   {
//     // std::string find_the_ultimate_answer(){
//     //   const auto url= "https://api.wolframalpha.com/v1/result?appid=YAPKJY-8XT9VEYPX9&i=what+is+ultimate+answer";
//     //   const auto curl = curl_easy_init();
//     //   curl_easy_setopt(curl, CURLOPT_URL, url);
//     //   curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
//     //   const auto write_func=[](char *ptr, size_t size, size_t nmemb, void *userdata){
//     //     auto & result= *static_cast<std::string*>(userdata);
//     //     result.append(ptr, size * nmemb);
//     //     return size * nmemb;
//     //   };
//     //   using WriteFunction = size_t(*)(char *, size_t, size_t, void*);
//     //   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, static_cast<WriteFunction>(write_func));
//     //   std::string result="";
//     //   curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
//     //   curl_easy_perform(curl);
//     //   curl_easy_cleanup(curl);
//     //   return result;
//     // }

//     std::string find_the_ultimate_answer() {
//             // 改成了调用 wolfram 库的 API，而不是使用 CURL 发请求
//             // 注：这里的 appid 是演示用的，只有免费的 2000 次/天调用额度，如有实际需要请自行申请
//             // return wolfram::simple_query("YAPKJY-8XT9VEYPX9", "what is the ultimate answer?");
//             return wolfram::simple_query(WOLFRAM_APPID, "what is the ultimate answer?");
//         }

//   }

// }

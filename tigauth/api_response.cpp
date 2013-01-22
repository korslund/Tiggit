#include "api_response.hpp"
#include <spread/misc/readjson.hpp>
#include <stdexcept>

using namespace TigAuth::API;

Response::Response(const std::string &str)
{
  clear();
  if(str != "")
    decode(str);
}

void Response::clear()
{
  isValid = false;
  isError = false;
  isAuthFail = false;
  hasUserInfo = false;
  newKey.clear();
  message.clear();
}

void Response::decode(const std::string &input)
{
  clear();

  // Get value as JSON
  Json::Value val = ReadJson::strToJson(input);

  // Get response type
  std::string str = val["type"].asString();

  if(str == "error") isError = true;
  else if(str == "authfail")
    {
      isError = true;
      isAuthFail = true;
    }
  else if(str == "message") {}
  else
    throw std::runtime_error("Unexpected server response: " + ((str=="")?input:str));

  // Decode other parts of the message

  if(val.isMember("newkey"))
    newKey = val["newkey"].asString();

  if(val.isMember("message"))
    message = val["message"].asString();

  if(val.isMember("generated"))
    generated = val["generated"].asString();

  if(val.isMember("userinfo"))
    {
      hasUserInfo = true;

      Json::Value usr = val["userinfo"];

      userid = usr["userid"].asString();
      usernick = usr["nickname"].asString();
      userauth = usr["authname"].asString();
      useritems = usr["items"].asString();
    }

  // If we get here, we have successfully decoded the response.
  isValid = true;
}

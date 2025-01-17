/* MODIFIED BY: rcook on Thu May 15 17:42:03 PDT 2014 */
/* VERSION: 1.0 */
#ifndef RDC_PATHUTIL_H
#define RDC_PATHUTIL_H
#include "stringutil.h"

#include <limits.h>
//===============================================================
/*!
  Returns the full path to the directory containing the given file or directory, without the trailing "/", unless the result is "/" itself.  
  Examples: 
  Dirname("/usr/local") == "/usr"
  Dirname("/usr/local/") == "/usr"
  Dirname("relative/path") == "/full/path/to/relative"
  Dirname("my/directory/") == "/full/path/to/my"
  Dirname("/") == "/"

*/ 
inline string Dirname(string filename) {
  string dirname = StripBack(filename, "/"); 

  if (!dirname.length()) return string("/");
 
  if (dirname[0] != '/')  {
    // need to switch to absolute path
    char wd[PATH_MAX]; // this is why I hate C programming -- there is no way to be sure about buffer overflows with this kind of crap lingering around
    getcwd(wd, PATH_MAX);

    string::size_type loc = dirname.find("/");
    if (loc == string::npos) {
      return string(wd); 
    }
    // prepend current working directory to mDirname
    dirname  = string(wd) + "/" + dirname; 
  } 
  string::size_type loc = dirname.rfind("/");
  if (loc == string::npos) { // reality check 
    throw string("Error in Dirname(): Logic error: there are no '/' chars in supposedly absolute path: ") + dirname; 
  }
  return  StripBack(dirname.substr(0,loc),"/"); 
}

//===============================================================
/*!
  Returns the last link of the filename, ala the "basename" shell command.  
*/ 
inline string Basename(string filename) {
  filename = StripBack(filename, "/"); 
  string::size_type loc = filename.rfind("/");
  if (loc == string::npos) {
    return filename; 
  } 
  string filename2 = filename.substr(loc+1); 
  
  return filename2; 
}
#endif

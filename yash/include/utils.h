//
// Created by Holden Nicoletti on 5/24/22.
//

#ifndef YASH_UTILS_H
#define YASH_UTILS_H

#include <string>

// Used to set Terminal Control to current process group
void getTC();

// Remove spaces at beginning and end of string
void trim(std::string&);

#endif //YASH_UTILS_H

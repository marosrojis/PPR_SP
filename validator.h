#pragma once
#ifndef __VALIDATOR_H__
#define __VALIDATOR_H__

#include <vector>
#include <iostream>
#include <sstream>
#include "structs.h"

config* validate_input(std::vector<std::string> &args);

#endif
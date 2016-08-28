// Copyright (c) 2009-2016 The Tao Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TAO_SUPPORT_VECTORS_H
#define TAO_SUPPORT_VECTORS_H

#include "util.h"
#include "json/json_spirit_writer_template.h"

using namespace json_spirit;
using namespace std;

std::string stringFromVch(const std::vector<unsigned char> &vch);
std::vector<unsigned char> vchFromValue(const Value& value);
std::vector<unsigned char> vchFromString(const std::string &str);
std::string stringFromValue(const Value& value);

#endif // TAO_SUPPORT_VECTORS_H
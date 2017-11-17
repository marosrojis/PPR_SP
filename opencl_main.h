#pragma once
#ifndef __OPENCL_MAIN_H__
#define __OPENCL_MAIN_H__

#include <stdio.h>
#include <codeanalysis\warnings.h>
#pragma warning( push )
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#include <CL/cl.hpp>
#pragma warning( pop )

void opencl_main_test();

#endif
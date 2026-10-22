/*
MeasurementValue.h

class MeasurementValue and enum MeasurementValidity.

Remark: Code is in .h file since this is the only way to have a single template implementation
in C++ (and not one for each combination of template parameters).

*/

#pragma once

#include "../../global_defines.h"

typedef enum {
    invalid = 0,
    valid = 1,
    computed = 2
} MeasurementValidity;

#if DEBUG_LEVEL >= 2
static const char* validities[] = {"invalid","valid","computed"};

inline const char* get_validity_string(MeasurementValidity validity){    
    return validities[validity];
}
#endif

template<typename MeasValueType> class MeasurementValue
{
    MeasValueType value;
    MeasurementValidity status;
public:
    MeasurementValue(){
        value = 0;
        status = invalid;
    }
    
    inline void set_value(MeasValueType val, MeasurementValidity status){
        value = val;
        this->status = status;
    }

    inline bool is_valid() {return status==valid;}
    inline bool is_computed() {return status==computed;}
    inline MeasurementValidity get_status() {return status;} 
    inline MeasValueType get_value() {return value;}
};
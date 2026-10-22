/*
SensorMeasurement.h

(Abstract) base class for all types of sensor measurement. The template parameters have to be 
specified by the inheriting classes; these are:
 - MeasValueType: Data Type of a single measurement value, e.g. float
 - MeasValueCount: The number of measurement values to be held in the buffer (typically equals 
   the number of values sampled in 10 minutes.
   
Remark 1: MeasValueCount needs to be a template parameter in order to be able to define the 
measurement value array (SensorMeasurment.value_buffer) statically, avoiding dynamic memory 
management.

Remark 2: Code is in .h file since this is the only way to have a single template implementation
in C++ (and not one for each combination of template parameters).

The class provides some base functionality which is called by the main program. This includes
all the management necessary for the value buffer; the inheriting sensor only needs to implement
the perform_measurement() method.
*/

#pragma once
#include "../../global_defines.h"

#include <Arduino.h>
#include "lmic.h"
#include <LoraMessage.h>
#include "../task/task.h"
#include "MeasurementValue.h"

template <typename MeasValueType, int MeasValueCount> class SensorMeasurement : public Task
{
protected:
    MeasurementValue<MeasValueType> value_buffer[MeasValueCount];   // Measurement value ring buffer
    int values_head, values_tail;                                   // Variables to control the ring buffer: values head points at the newest, and values_tail at the oldest element in the buffer. If values_head == -1, this indicates that the buffer is empty.
    String name_sensor;
public:
    SensorMeasurement(String name_sensor, uint64_t interval_ms):Task(interval_ms){
        this->name_sensor = name_sensor;
        this->reset();
    }

    void reset(){
        // Setting head to -1 signals that the queue is empty:
        values_head = -1;
        values_tail = 0;
    }

    int get_number_values(){
        if (values_head==-1) return 0;                                                              // no element in ring buffer <=> values_head == -1
        else if (values_head<values_tail) return (values_head+1) + (MeasValueCount-values_tail);    // values_head < values_tail: The ring buffer starts at the end of the 
                                                                                                    // array and continues at the beginning, so sum up the number of elements   
                                                                                                    // from the beginning of the array to the tail and from the head to the 
                                                                                                    // end of the array.
        else return values_head-values_tail;        // the "easy" case: ring buffer starts at a tail position smaller than the head position
    }

    float get_variance(){
        int number_values = get_number_values();
        if (number_values == 0) return NAN;

        float mean = get_mean();
        float sum = 0, count = 0;
        for (int i=values_tail; i<number_values; i++){
            int pos = i % MeasValueCount;                   // compute the correct position in the ring buffer
            if (value_buffer[pos].is_valid()){
                sum += pow(value_buffer[pos].get_value() - mean, 2);
                count += 1;
            }            
        }
        return sum/count;
    }

    float get_mean(){
        int number_values = get_number_values();
        if (number_values == 0) return NAN;
        
        float sum = 0, count = 0;
        for (int i=values_tail; i<number_values; i++){
            int pos = i % MeasValueCount;                // compute the correct position in the ring buffer
            if (value_buffer[pos].is_valid()){
                sum += value_buffer[pos].get_value();
                count += 1;
            }            
        }
        return sum/count;
    }

    float get_min(bool raw=false){                      // Parameter raw determines whether invalid values shall be considered (when raw = true)
        int number_values = get_number_values();
        if (number_values == 0) return NAN;
        
        float min = 0;
        bool found_value = false;

        for (int i=values_tail; i<number_values; i++){
            int pos = i % MeasValueCount;               // compute the correct position in the ring buffer
            if (value_buffer[pos].is_valid() || raw==true){
                MeasValueType val = value_buffer[pos].get_value();
                if (val<min || found_value == false) {
                    min = val;
                    found_value = true;
                }
            }            
        }
        return min;
    }

    float get_max(bool raw=false){                      // Parameter raw determines whether invalid values shall be considered (when raw = true)
        int number_values = get_number_values();
        if (number_values == 0) return NAN;
        
        float max = 0;
        bool found_value = false;

        for (int i=values_tail; i<number_values; i++){
            int pos = i % MeasValueCount;               // compute the correct position in the ring buffer
            if (value_buffer[pos].is_valid() || raw==true){
                MeasValueType val = value_buffer[pos].get_value();
                if (val>max || found_value == false){
                    max = val;
                    found_value = true;
                }
            }            
        }
        return max;
    }

    float get_raw_min(){
        return get_min(true);
    }

    float get_raw_max(){
        return get_max(true);
    }

    void get_quartiles(MeasValueType* val_result_buffer[3]){
        // ToDo
    }

    void get_histogram(int *hist_result_buffer[], int hist_buffer_size){
        // ToDo
    }
        

protected:
    virtual void perform_measurement(MeasurementValue<MeasValueType> *val_buffer){
        // virtual, please override in derived class
        val_buffer->set_value(0,invalid);
    }

    virtual bool check_buffer_is_valid(){
        // Standard rule: Buffer is valid if at least 50% of the values are valid, and buffer is filled >= 50%
        if (values_head == -1) return false;    // no values in buffer => not valid

        // Count number of valid values in buffer by first counting from tail to head (inclusive) (or to the end of the buffer, if tail<head)
        int number_valid = 0;    
        for (int i=values_tail; i<=min(values_head,MeasValueCount-1); i++)
            if (value_buffer[i].is_valid()) number_valid++;

        // If head < tail, also count from 0 to head (inclusive)
        if (values_head < values_tail)
            for (int i=0; i<=values_head; i++)
                if (value_buffer[i].is_valid()) number_valid++;

        // Apply the above rule to determine the return value:
        int number_total = this->get_number_values();
        if (number_total*2 >= MeasValueCount && number_valid*2 >= number_total) return true;
        else return false;
    }

    virtual void buffer_interpolate_invalid_measurements(){
        // virtual, please override in derived class
    }  

    void add_measurement_value(MeasValueType val, MeasurementValidity validity){
        /* Adds a new measurement to the buffer and adjusts head and tail accordingly
           We first compute the new head position. If necessary (i.e., the buffer is full and the oldest element has to be overwritten), 
           the tail is also adapted, so that it will afterwards again point to the oldest element in the buffer. */

        if (values_head == -1){     // empty buffer => next head and tail shall be 0
            values_head = 0;
            values_tail = 0;
        }
        if (values_head>=MeasValueCount-1){     // "Overflow": Buffer is full (head has already reached the last index, MeasValueCount-1), we move the head to the beginning of the buffer hence (index 0). The old tail value was 0 in this case and needs to increase to 1.
            values_head = 0;                    
            values_tail = 1;
        }
        else if (values_head<values_tail){      // "Overflow" has happend previously, so head < tail (this actually means tail = head+1) => we need to increase both head and tail, i.e.: One element at the end of the buffer (old tail) is deleted and replaced by the new head element.
            values_head += 1;
            values_tail += 1;
            if (values_tail >= MeasValueCount) values_tail = 0;     // If the new tail has reached the end of the buffer, reset it to 0 => this is correct, because now the element at index 0 is the oldest one in the buffer.
        }
        else values_head +=1;                   // Buffer not yet full, simply increase head, tail will remain at 0.

        value_buffer[values_head].set_value(val,validity);      // Write the new measurement value at the new head position
    }

    virtual void run(){
        /* standard behaviour of a SensorMeasurement's run method: Call perform_measurement()
           once, and store the retrieved value + status in the buffer */

        static MeasurementValue<MeasValueType> value;   // static: only created once, not stored on stack
        perform_measurement(&value);
        add_measurement_value(value.get_value(),value.get_status());
        DEBUG(timer->get_current_time_ms());DEBUG(": Sensor "); DEBUG(name_sensor); DEBUG(" hat folgenden Wert gemessen: "); DEBUG(value.get_value()); DEBUG(" / "); DEBUGLN(get_validity_string(value.get_status()));
    }
};
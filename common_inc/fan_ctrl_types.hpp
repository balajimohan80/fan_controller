#ifndef __FAN_CTRL_TYPES_HPP__
#define __FAN_CTRL_TYPES_HPP__
using float32_t          = float;
using temp_DegC_t        = float32_t;
using max_PWM_Count_t    = uint32_t;
using curr_PWM_Count_t   = uint32_t; 
using pwm_Duty_t         = float32_t;          
using linera_InterPol_t  = std::pair<temp_DegC_t, pwm_Duty_t>;

using json_data_t = std::pair<float32_t, uint32_t>;

#endif

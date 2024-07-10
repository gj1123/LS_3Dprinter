
/**
  planner.h - buffers movement commands and manages the acceleration profile plan
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

// This module is to be considered a sub-module of stepper.c. Please don't include
// this file from any other module.



#ifndef __PLANNER_H
#define __PLANNER_H


#include "public.h"

int plan_init(void);


// 规划路径
void plan_buffer_line(FLOAT dx_position, 
                      FLOAT dy_position, 
                      FLOAT dz_position, 
                      FLOAT de_position, 
                      FLOAT feedrate_mm_sec,
                      unsigned char check_endstop);


// 根据输入的xyze绝对坐标修改相关变量
void plan_set_position(float x_abs_coordinate_mm,
                       float y_abs_coordinate_mm,
                       float z_abs_coordinate_mm,
                       float e_abs_coordinate_mm);



#endif




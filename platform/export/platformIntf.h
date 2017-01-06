/*
 * platformIntf.h
 *
 *  Created on: Jan 4, 2017
 *      Author: erik
 */

#ifndef EXPORT_PLATFORMINTF_H_
#define EXPORT_PLATFORMINTF_H_

typedef enum
{
  Pin_Relais_Unknown = 0,
  PIN_Relais_1 = 1,
  PIN_Relais_2 =2 ,
} Relais_e;

typedef enum
{
  Direction_Unknown = 0,
  Direction_Input = 1,
  Direction_Output =2,
} PinDirection_e;

typedef struct _platform_interface
{


} IPlatform_t;






#endif /* EXPORT_PLATFORMINTF_H_ */

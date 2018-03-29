#include "s3c2416.h"

int set_motor_dir (int32_t motor_id, int32_t motor_dir)
{
	switch (motor_id){
		case 0:
			break;
		case 2:
			MOTOR2_DIR(motor_dir);  //
			break;
		default:break;
	}
	return 0;
}
int start_motor (int32_t motor_id, int32_t motor_step, int32_t motor_speed)
{
	int i;
	//cy_println ("run motor id = %d", motor_id);
	switch (motor_id){
		case 2:
			for (i = 0; i < motor_step; i++){
				MOTOR2(STARTRUN);	  //  
				time = motor_speed;	  //
				while(time != 0);
				MOTOR2 (STOPRUN);	  //  
				time = motor_speed;	  //
				while(time != 0);
				if (had_ctrlc ()){
					return -1;
				}
			}
			break;
		default:
		    return -1;
			break;
	}
	return 0;
}

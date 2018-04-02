#include "s3c2416.h"

s_motor_def motors[MOTOR_NUMS];

int set_motor_dir (int32_t motor_id, int32_t motor_dir)
{
	switch (motor_id){
		case 0:
			MOTOR0_DIR(motor_dir);  //
			motors[0].motor_dir = motor_dir;
			break;
		case 1:
			MOTOR1_DIR(motor_dir);  //
			motors[1].motor_dir = motor_dir;
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
//	int i;
	//cy_println ("run motor id = %d", motor_id);
	switch (motor_id){
		case 0:
			motors[0].motor_pwm_status = MOTOR_PWM_HIGH;
			motors[0].motor_spd = motor_speed;
			motors[0].motor_steps = motor_step;
			if (motors[0].motor_dir == MOTOR_DIR_UP){
				motors[0].motor_watch = MOTOR_WATCH_POS1;
			}else{
				motors[0].motor_watch = MOTOR_WATCH_POS2;
			}
			motors[0].motor_status = MOTOR_RUN;
//			for (i = 0; i < motor_step; i++){
//				MOTOR0(STARTRUN);	  //  
//				time = motor_speed;	  //
//				while(time != 0);
//				MOTOR0 (STOPRUN);	  //  
//				time = motor_speed;	  //
//				while(time != 0);
//				if (had_ctrlc ()){
//					return -1;
//				}
//			}
			break;
		case 1:
			motors[1].motor_pwm_status = MOTOR_PWM_HIGH;
			motors[1].motor_spd = motor_speed;
			motors[1].motor_steps = motor_step;
			if (motors[1].motor_dir == MOTOR_DIR_OPEN){
				motors[1].motor_watch = MOTOR_WATCH_POS1;
			}else{
				motors[1].motor_watch = MOTOR_WATCH_POS2;
			}
			motors[1].motor_status = MOTOR_RUN;
//			for (i = 0; i < motor_step; i++){
//				MOTOR1(STARTRUN);	  //  
//				time = motor_speed;	  //
//				while(time != 0);
//				MOTOR1 (STOPRUN);	  //  
//				time = motor_speed;	  //
//				while(time != 0);
//				if (had_ctrlc ()){
//					return -1;
//				}
//			}
			break;
		case 2:
			motors[2].motor_pwm_status = MOTOR_PWM_HIGH;
			motors[2].motor_spd = motor_speed;
			motors[2].motor_steps = motor_step;
			motors[2].motor_status = MOTOR_RUN;
//			for (i = 0; i < motor_step; i++){
//				MOTOR2(STARTRUN);	  //  
//				time = motor_speed;	  //
//				while(time != 0);
//				MOTOR2 (STOPRUN);	  //  
//				time = motor_speed;	  //
//				while(time != 0);
//				if (had_ctrlc ()){
//					return -1;
//				}
//			}
			break;
		default:
		    return -1;
			//break;
	}
	return 0;
}


void set_motor(int32_t motor_id, int32_t motor_dir, int32_t steps, int32_t speed, int32_t watch)
{
	if (motor_id > MOTOR_NUMS - 1)
		return;
	if (motors[motor_id].motor_status == MOTOR_RUN){
		motors[motor_id].motor_steps += steps;
	}else{
		set_motor_dir (motor_id,  motor_dir);
		motors[motor_id].motor_pwm_status = MOTOR_PWM_HIGH;
		motors[motor_id].motor_steps = steps;
		motors[motor_id].motor_spd = speed;
		motors[motor_id].motor_watch = watch;
		if (motors[motor_id].motor_steps > 0){
			motors[motor_id].motor_status = MOTOR_RUN;
		}else{
			motors[motor_id].motor_status = MOTOR_STOP;
		}
	}
}

void set_motor_cplt_callback (int32_t motor_id, motor_cplt_call_back cb)
{
	if (motor_id > MOTOR_NUMS - 1)
		return;
	motors[motor_id].cb = cb;
}
void clr_motor_cplt_callback (int32_t motor_id)
{
	if (motor_id > MOTOR_NUMS - 1)
		return;
	motors[motor_id].cb = 0;
}


void motor_0_cplt_callback (void)
{
	set_motor (0, MOTOR_DIR_UP, 300, 6, MOTOR_WATCH_NONE);
}
void motor_1_cplt_callback (void)
{
	set_motor (1, MOTOR_DIR_CLOSE, para_set_value.data.coin_size, 8, MOTOR_WATCH_NONE);
}
void motor_2_cplt_callback (void)
{
}



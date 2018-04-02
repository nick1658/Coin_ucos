#ifndef MOTOR_H
#define MOTOR_H


#define MOTOR0(x) OUT0(x)
#define MOTOR1(x) OUT2(x)
#define MOTOR2(x) OUT4(x)
#define MOTOR3(x) //OUT3(x)
#define MOTOR4(x) //OUT4(x)
#define MOTOR5(x) //OUT5(x)
#define MOTOR6(x) //OUT6(x)
#define MOTOR7(x) //OUT7(x)



#define MOTOR0_DIR(x) OUT1(x)
#define MOTOR1_DIR(x) OUT3(x)
#define MOTOR2_DIR(x) OUT5(x)
#define MOTOR3_DIR(x) //OUT3(x)
#define MOTOR4_DIR(x) //OUT4(x)
#define MOTOR5_DIR(x) //OUT5(x)
#define MOTOR6_DIR(x) //OUT6(x)
#define MOTOR7_DIR(x) //OUT7(x)


typedef enum {
	MOTOR_STOP= 1,
	MOTOR_RUN = 2,
	MOTOR_WATCH_POS1 = 3,
	MOTOR_WATCH_POS2 = 4,
	MOTOR_WATCH_POS3 = 5,
	MOTOR_WATCH_NONE = 6,
	MOTOR_PWM_LOW = 7,
	MOTOR_PWM_HIGH = 8,
} e_motor_status;

#define MOTOR_DIR_UP 0
#define MOTOR_DIR_DOWN 1
#define MOTOR_DIR_ROLL 1
#define MOTOR_DIR_NONE 1

#define MOTOR_DIR_CLOSE 0
#define MOTOR_DIR_OPEN 1

typedef void (*motor_cplt_call_back) (void);


typedef struct {
	int32_t motor_id;
	int32_t motor_status;
	int32_t motor_steps;
	int32_t motor_dir;
	int32_t motor_spd;
	int32_t motor_spd_ctr;
	int32_t motor_watch;
	int32_t motor_pwm_status;
	motor_cplt_call_back cb;
}s_motor_def;

#define MOTOR_NUMS 3


#define EXC_MOTOR(X) { \
	if ((motors[X].motor_steps > 0)){ \
		motors[X].motor_spd_ctr++; \
		if (motors[X].motor_pwm_status == MOTOR_PWM_HIGH){ \
			if (motors[X].motor_spd_ctr >= motors[X].motor_spd){ \
				motors[X].motor_spd_ctr = 0; \
				MOTOR##X(STARTRUN);	   \
				motors[X].motor_pwm_status = MOTOR_PWM_LOW; \
			} \
		}else{ \
			if (motors[X].motor_spd_ctr >= motors[X].motor_spd){ \
				motors[X].motor_spd_ctr = 0; \
				MOTOR##X(STOPRUN); \
				motors[X].motor_pwm_status = MOTOR_PWM_HIGH; \
				motors[X].motor_steps--; \
			} \
		} \
	}else{ \
		motors[X].motor_status = MOTOR_STOP; \
		if (motors[X].cb != 0){ \
			motors[X].cb (); \
			motors[X].cb = 0; \
		} \
	} \
}


#define SET_MOTOR_CPLT_CALLBACK(X) { \
	if (X < MOTOR_NUMS){ \
		motors[X].cb = motor_##X##_cplt_callback; \
	} \
}

extern s_motor_def motors[MOTOR_NUMS];

void set_motor(int32_t motor_id, int32_t motor_dir, int32_t steps, int32_t speed, e_motor_status watch);
int start_motor (int32_t motor_id, int32_t motor_step, int32_t motor_speed);
int set_motor_dir (int32_t motor_id, int32_t motor_dir);
void set_motor_cplt_callback (int32_t motor_id, motor_cplt_call_back cb);
void clr_motor_cplt_callback (int32_t motor_id);
void motor_0_cplt_callback (void);
void motor_1_cplt_callback (void);
void motor_2_cplt_callback (void);

#endif



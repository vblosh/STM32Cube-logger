#ifndef INC_LOGGER_LOCKS_H_
#define INC_LOGGER_LOCKS_H_

void lock(uint32_t* interrupts_enabled);
void unlock(uint32_t* interrupts_enabled);

#endif /* INC_LOGGER_LOCKS_H_ */

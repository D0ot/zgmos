#ifndef __RWV_H_
#define __RWV_H_

#define RWV32(x) (* ( (volatile uint32_t*)(&(x)) ) )
#define RWV8(x) (* ( (volatile uint8_t*)(&(x)) ) )

#endif // __RWV_H_

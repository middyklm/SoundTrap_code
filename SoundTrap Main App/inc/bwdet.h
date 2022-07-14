/*
 * bwdet.h
 *
 *  Created on: 23/11/2015
 *      Author: jatk009
 */

#ifndef BWDET_H_
#define BWDET_H_

int BWDET_open(long fs, CDET_Params * params);
int		BWDET_close(void);
void	BWDET_state(int on);
void	BWDET_flush(void);

#endif /* BWDET_H_ */

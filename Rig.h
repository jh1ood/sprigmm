/*
 * Rig.h
 *
 *  Created on: Jul 20, 2015
 *      Author: user1
 */

#ifndef RIG_H_
#define RIG_H_

#include "RigParams.h"

class Rig : public RigParams {
public:
	virtual int   get_frequency     ()    = 0;
	virtual void  set_frequency     (int) = 0;
	virtual int   get_operating_mode()    = 0;
	virtual void  set_operating_mode(int) = 0;
	virtual ~Rig();
};

#endif /* RIG_H_ */

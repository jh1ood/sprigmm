/*
 * RigSoft66.h
 *
 *  Created on: Jul 20, 2015
 *      Author: user1
 */

#ifndef RIGSOFT66_H_
#define RIGSOFT66_H_

#include "Rig.h"

class RigSoft66 : public Rig {
public:
	RigSoft66(char *s);

	int  get_frequency      ()    override { return frequency;}
	void set_frequency      (int) override { ; }
	int  get_operating_mode ()    override { return 0;}
	void set_operating_mode(int) override { ; }

	virtual ~RigSoft66();
private:
};

#endif /* RIGSOFT66_H_ */

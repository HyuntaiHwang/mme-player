#ifndef MMP_OAL_COND_LINUX_HPP__
#define MMP_OAL_COND_LINUX_HPP__

#include "mmp_oal_cond.hpp"

#if (MMP_OS == MMP_OS_LINUX)

class mmp_oal_cond_linux : public mmp_oal_cond {

friend class mmp_oal_cond;

private:
	pthread_cond_t mCond;
	
protected:
    
	mmp_oal_cond_linux();
	virtual ~mmp_oal_cond_linux();

	virtual MMP_ERRORTYPE open();
	virtual MMP_ERRORTYPE close();

public:
	virtual void signal();
	virtual void wait(class mmp_oal_mutex* p_mutex);	
};


#endif 
#endif

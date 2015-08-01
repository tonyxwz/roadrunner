// == PREAMBLE ================================================

// * Licensed under the Apache License, Version 2.0; see README

// == FILEDOC =================================================

/** @file IntegratorRegistration.cpp
* @author JKM
* @date 07/31/2015
* @copyright Apache License, Version 2.0
* @brief Registers the integrators at startup
**/

// == INCLUDES ================================================

# include "rrConfigure.h"
# include "IntegratorRegistration.h"
# include "Integrator.h"
# include "CVODEIntegrator.h"
# include "GillespieIntegrator.h"
# include "RK4Integrator.h"
# include "EulerIntegrator.h"

# if RR_USE_CXX11
#   include <mutex>
# endif
// == CODE ====================================================

namespace rr
{
    // call exactly once
    static void register_integrators_at_init() {
        IntegratorFactory::getInstance().registerIntegrator(new CVODEIntegratorRegistrar());
        IntegratorFactory::getInstance().registerIntegrator(new GillespieIntegratorRegistrar());
    }

    void IntegratorRegistrationMgr::Register() {
# if RR_USE_CXX11
        static std::once_flag flag;
        std::call_once(flag, register_integrators_at_init());
# else
        // WARNING: not thread safe
        static bool flag = false;
        if (!flag) {
            flag = true;
            register_integrators_at_init();
        }
# endif
    }
}
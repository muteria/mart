#ifndef __KLEE_SEMU_GENMU_operatorClasses__MemoryAddressValue__
#define __KLEE_SEMU_GENMU_operatorClasses__MemoryAddressValue__

/**
 * -==== MemoryAddressValue.h
 *
 *                MuLL Multi-Language LLVM Mutation Framework
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details. 
 *  
 * \brief     Matching and replacement mutation operator for memory addresses.
 * \details   This abstract class define is extended from the @see MemoryPointerAddress_Base class. 
 */
 
#include "../MemoryPointerAddress_Base.h"

class MemoryAddressValue: public MemoryPointerAddress_Base
{
  protected:
    /**
     * \brief Implement from @see MemoryPointerAddress_Base: Match memory address
     */
    inline bool isMatched(llvm::Value *val)
    {  
        return val->getType()->isPointerTy();
    }
};

#endif //__KLEE_SEMU_GENMU_operatorClasses__MemoryAddressValue__
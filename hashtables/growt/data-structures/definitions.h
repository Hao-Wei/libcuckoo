/*******************************************************************************
 * data-structures/definitions.h
 *
 * Convenience file that defines some often used hash table variants.
 *
 * Part of Project growt - https://github.com/TooBiased/growt.git
 *
 * Copyright (C) 2015-2016 Tobias Maier <t.maier@kit.edu>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include "seqcircular.h"

#include "simpleelement.h"
#include "markableelement.h"
#include "circular.h"
#include "strategy/wstrat_user.h"
#include "strategy/wstrat_pool.h"
#include "strategy/estrat_async.h"
#include "strategy/estrat_sync.h"
#include "strategy/estrat_sync_alt.h"
#include "growtable.h"

namespace growt {

template<class HashFct = std::hash<size_t>, class Allocator = std::allocator<char> >
using SequentialTable = SeqCircular<SimpleElement, HashFct, Allocator>;


template<class HashFct = std::hash<typename SimpleElement::Key>, class Allocator = std::allocator<char> >
using folklore    = Circular<SimpleElement, HashFct, Allocator>;


template<class E, class HashFct = std::hash<E>, class Allocator = std::allocator<E> >
using NoGrow      = Circular<E, HashFct, Allocator>;

template<class HashFct    = std::hash<typename MarkableElement::Key>,
         class Allocator  = std::allocator<char> >
using uaGrow  = GrowTable<NoGrow<MarkableElement, HashFct, Allocator>, WStratUser, EStratAsync>;

template<class HashFct    = std::hash<typename SimpleElement::Key>,
         class Allocator  = std::allocator<char> >
using usGrow  = GrowTable<NoGrow<MarkableElement, HashFct, Allocator>, WStratUser, EStratSync>;

template<class HashFct    = std::hash<typename SimpleElement::Key>,
         class Allocator  = std::allocator<char> >
using usnGrow = GrowTable<NoGrow<MarkableElement, HashFct, Allocator>, WStratUser, EStratSyncNUMA>;


template<class HashFct    = std::hash<typename MarkableElement::Key>,
         class Allocator  = std::allocator<char> >
using paGrow  = GrowTable<NoGrow<MarkableElement, HashFct, Allocator>, WStratPool, EStratAsync>;

template<class HashFct    = std::hash<typename SimpleElement::Key>,
         class Allocator  = std::allocator<char> >
using psGrow  = GrowTable<NoGrow<MarkableElement, HashFct, Allocator>, WStratPool, EStratSync>;

template<class HashFct   = std::hash<typename SimpleElement::Key>,
         class Allocator = std::allocator<char> >
using psnGrow = GrowTable<NoGrow<MarkableElement, HashFct, Allocator>, WStratPool, EStratSyncNUMA>;

}

#endif // DEFINITIONS_H

/*
 * Copyright (c) 2018 Andrew Savonichev <andrew.savonichev@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOOPENCLCOMPOSITEOPALPHADARKEN32_H_
#define KOOPENCLCOMPOSITEOPALPHADARKEN32_H_

#include "KoCompositeOpBase.h"
#include "KoCompositeOpRegistry.h"
#include <klocalizedstring.h>
#include "KoStreamedMath.h"

/**
 * An OpenCL version of a composite op for the use in 4 byte
 * colorspaces with alpha channel placed at the last byte of
 * the pixel: C1_C2_C3_A.
 */
class KoOpenCLCompositeOpAlphaDarken32 : public KoCompositeOp
{
public:
    KoOpenCLCompositeOpAlphaDarken32(const KoColorSpace* cs)
        : KoCompositeOp(cs, COMPOSITE_ALPHA_DARKEN, i18n("Alpha darken"), KoCompositeOp::categoryMix()) {}

    using KoCompositeOp::composite;

    virtual void composite(const KoCompositeOp::ParameterInfo& params) const
    {
    }
};

#endif // KOOPENCLCOMPOSITEOPALPHADARKEN32_H_

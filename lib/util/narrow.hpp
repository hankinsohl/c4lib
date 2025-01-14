// The implementation of narrow is taken from the Microsoft GSL.  The Microsoft GSL
// is copyright as follows:
///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <lib/util/exception-formats.hpp>
#include <stdexcept>

namespace gsl {

template<class T, class U> constexpr T narrow(U u) noexcept(false)
{
    constexpr bool is_different_signedness{std::is_signed_v<T> != std::is_signed_v<U>};

    T t{static_cast<T>(u)};
    if (static_cast<U>(t) != u || (is_different_signedness && ((t < T{}) != (u < U{})))) {
        throw std::logic_error(c4lib::fmt::narrowing_error);
    }

    return t;
}

} // namespace gsl

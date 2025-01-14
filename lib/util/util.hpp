// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/10/2024.

#pragma once

namespace c4lib {

template<typename T> const T& unmove(T&& x)
{
    return x;
}

} // namespace c4lib

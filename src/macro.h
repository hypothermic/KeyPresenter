//
// Created by xforce on 05-05-20.
//

#ifndef KEYPRESENTER_MACRO_H
#define KEYPRESENTER_MACRO_H

#ifdef __GNUC__
#    define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#    define UNUSED(x) UNUSED_ ## x
#endif

#endif //KEYPRESENTER_MACRO_H

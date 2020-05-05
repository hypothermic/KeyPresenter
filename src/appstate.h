#ifndef KEYPRESENTER_APPSTATE_H
#define KEYPRESENTER_APPSTATE_H

#include <glib.h>

#define APP_STATE(app_state) (((AppState*) app_state))

typedef struct _AppState AppState;

struct _AppState {
    gboolean screen_supports_alpha_channel;
};

#endif //KEYPRESENTER_APPSTATE_H

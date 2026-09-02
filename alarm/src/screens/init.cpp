#include "../../include/screens.h"
#include "../../include/lcd_wrapper.h"
#include "../../include/config.h"

void screen_init_enter(Context* ctx) {
    lcd_clear();
    lcd_print_line(0, "  Alarm Clock   ");
    lcd_print_line(1, "  Starting...   ");
    delay(1200);
    lcd_clear();

    // Transition to clock screen
    ctx->state        = STATE_CLOCK;
    ctx->needs_redraw = true;
}

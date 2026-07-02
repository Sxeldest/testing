package com.xyron.game.launcher.util;

import android.content.Context;
import android.content.SharedPreferences;

public class DataVariantPreferences {
    private static final String PREFS_NAME = "data_variant_prefs";
    private static final String KEY_SELECTED_VARIANT = "selected_variant_id";
    public static final String DATA_VARIANT_LITE = "lite";
    public static final String DATA_VARIANT_FULL = "full";

    public static void saveSelectedVariantId(Context context, String variantId) {
        context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
                .edit().putString(KEY_SELECTED_VARIANT, variantId).apply();
    }

    public static String getSelectedVariantId(Context context) {
        return context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
                .getString(KEY_SELECTED_VARIANT, "");
    }
}

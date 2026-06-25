/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * This file is part of PvZ-Portable.
 *
 * PvZ-Portable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PvZ-Portable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PvZ-Portable. If not, see <https://www.gnu.org/licenses/>.
 */

package io.github.wszqkzqk.pvzportable;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.WindowManager;

import org.libsdl.app.SDLActivity;

import java.io.File;

public class PvZPortableActivity extends SDLActivity {
    private static final String TAG = "PvZPortable";
    
    private static final long DOUBLE_CLICK_TIMEOUT = 300; // 双击超时时间（毫秒）
    private long lastVolumeKeyTime = 0;
    private int lastVolumeKeyCode = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        File extDir = getExternalFilesDir(null);
        if (extDir != null && !extDir.exists()) extDir.mkdirs();
        Log.i(TAG, "Resource dir: " + (extDir != null ? extDir.getAbsolutePath() : "null"));

        if (!hasGameResources(extDir)) {
            super.onCreate(savedInstanceState);
            startActivity(new Intent(this, ResourceImportActivity.class));
            finish();
            return;
        }

        super.onCreate(savedInstanceState);
        hideSystemUI();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) hideSystemUI();
    }

    private void hideSystemUI() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            Window window = getWindow();
            window.setDecorFitsSystemWindows(false);
            WindowInsetsController controller = window.getInsetsController();
            if (controller != null) {
                controller.hide(WindowInsets.Type.statusBars() | WindowInsets.Type.navigationBars());
                controller.setSystemBarsBehavior(
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else {
            getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
        }
    }
    
    @Override
    public boolean dispatchKeyEvent(android.view.KeyEvent event) {
        int keyCode = event.getKeyCode();
        
        // 检查是否是音量键的按下事件，并且不是重复事件（过滤长按）
        if (event.getAction() == android.view.KeyEvent.ACTION_DOWN && 
            event.getRepeatCount() == 0 && 
            (keyCode == android.view.KeyEvent.KEYCODE_VOLUME_UP || keyCode == android.view.KeyEvent.KEYCODE_VOLUME_DOWN)) {
            long currentTime = System.currentTimeMillis();
            
            // 检查是否是双击
            if (keyCode == lastVolumeKeyCode && (currentTime - lastVolumeKeyTime) < DOUBLE_CLICK_TIMEOUT) {
                // 双击音量键，打开输入键盘
                SDLActivity.showTextInput(0, 0, 0, 0);
                return true; // 消费事件，防止重复处理
            }
            
            // 记录本次按键信息
            lastVolumeKeyCode = keyCode;
            lastVolumeKeyTime = currentTime;
        }
        
        // 调用父类方法，保持正常的音量调节功能
        return super.dispatchKeyEvent(event);
    }

    @Override
    protected String[] getLibraries() {
        return new String[]{
            "SDL2",
            "main"
        };
    }

    @Override
    public void setRequestedOrientation(int requestedOrientation) {
        if (requestedOrientation == android.content.pm.ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE) {
            requestedOrientation = android.content.pm.ActivityInfo.SCREEN_ORIENTATION_USER_LANDSCAPE;
        }
        super.setRequestedOrientation(requestedOrientation);
    }

    private static boolean hasGameResources(File dir) {
        if (dir == null || !dir.isDirectory()) return false;
        File pak = new File(dir, "main.pak");
        File props = new File(dir, "properties");
        return pak.exists() && props.isDirectory();
    }
}

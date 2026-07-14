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
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.documentfile.provider.DocumentFile;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

/**
 * Resource import and save data management via SAF.
 * No runtime storage permissions required.
 */
public class ResourceImportActivity extends AppCompatActivity {
    private static final String TAG = "ResImport";
    private static final int BUFFER_SIZE = 8192;

    private File gameDir;
    private TextView statusText;
    private ProgressBar progressBar;
    private Button btnPickZip;
    private Button btnPickDir;
    private Button btnExportSave;
    private Button btnImportSaveZip;
    private Button btnImportSaveDir;
    private Button btnLaunchGame;

    private final ActivityResultLauncher<String[]> zipPicker =
        registerForActivityResult(new ActivityResultContracts.OpenDocument(), uri -> {
            if (uri != null) importFromZip(uri);
        });

    private final ActivityResultLauncher<Uri> dirPicker =
        registerForActivityResult(new ActivityResultContracts.OpenDocumentTree(), uri -> {
            if (uri != null) importFromDirectory(uri);
        });

    private final ActivityResultLauncher<String> saveExporter =
        registerForActivityResult(new ActivityResultContracts.CreateDocument("application/zip"), uri -> {
            if (uri != null) exportSaveData(uri);
        });

    private final ActivityResultLauncher<String[]> saveZipImporter =
        registerForActivityResult(new ActivityResultContracts.OpenDocument(), uri -> {
            if (uri != null) importSaveDataFromZip(uri);
        });

    private final ActivityResultLauncher<Uri> saveDirImporter =
        registerForActivityResult(new ActivityResultContracts.OpenDocumentTree(), uri -> {
            if (uri != null) importSaveDataFromDir(uri);
        });

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_resource_import);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(android.R.id.content), (v, insets) -> {
            Insets bars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(bars.left, bars.top, bars.right, bars.bottom);
            return insets;
        });

        gameDir = getExternalFilesDir(null);
        if (gameDir != null && !gameDir.exists()) gameDir.mkdirs();

        statusText    = findViewById(R.id.status_text);
        progressBar   = findViewById(R.id.progress_bar);
        btnPickZip    = findViewById(R.id.btn_pick_zip);
        btnPickDir    = findViewById(R.id.btn_pick_dir);
        btnExportSave = findViewById(R.id.btn_export_save);
        btnImportSaveZip = findViewById(R.id.btn_import_save_zip);
        btnImportSaveDir = findViewById(R.id.btn_import_save_dir);
        btnLaunchGame = findViewById(R.id.btn_launch_game);

        btnPickZip.setOnClickListener(v ->
            zipPicker.launch(new String[]{"application/zip", "application/x-zip-compressed"})
        );
        btnPickDir.setOnClickListener(v ->
            dirPicker.launch(null)
        );
        btnExportSave.setOnClickListener(v ->
            saveExporter.launch("pvz-portable-savedata.zip")
        );
        btnImportSaveZip.setOnClickListener(v ->
            saveZipImporter.launch(new String[]{"application/zip", "application/x-zip-compressed"})
        );
        btnImportSaveDir.setOnClickListener(v ->
            saveDirImporter.launch(null)
        );
        btnLaunchGame.setOnClickListener(v -> launchGame());

        refreshStatus();
    }

    private boolean hasResources() {
        if (gameDir == null) return false;
        File pak = new File(gameDir, "main.pak");
        File props = new File(gameDir, "properties");
        return pak.exists() && props.isDirectory();
    }

    private boolean hasSaveData() {
        if (gameDir == null) return false;
        File save = new File(gameDir, "userdata");
        return save.isDirectory() && save.list() != null && save.list().length > 0;
    }

    private void refreshStatus() {
        boolean ready = hasResources();
        if (ready) {
            statusText.setText(R.string.status_ready);
            btnLaunchGame.setEnabled(true);
        } else {
            statusText.setText(R.string.status_missing);
            btnLaunchGame.setEnabled(false);
        }
        btnExportSave.setEnabled(hasSaveData());
        progressBar.setVisibility(View.GONE);
    }

    private void launchGame() {
        Intent intent = new Intent(this, PvZPortableActivity.class);
        startActivity(intent);
        finish();
    }

    private void importFromZip(Uri uri) {
        setWorking(true);
        new Thread(() -> {
            try (InputStream is = getContentResolver().openInputStream(uri);
                 ZipInputStream zis = new ZipInputStream(is)) {
                ZipEntry entry;
                while ((entry = zis.getNextEntry()) != null) {
                    if (entry.isDirectory()) {
                        zis.closeEntry();
                        continue;
                    }
                    String name = stripCommonPrefix(entry.getName());
                    if (name == null) { zis.closeEntry(); continue; }

                    File outFile = resolveWithinGameDir(new File(gameDir, name));
                    File parent = outFile.getParentFile();
                    if (parent != null && !parent.exists()) parent.mkdirs();

                    try (OutputStream os = new BufferedOutputStream(new FileOutputStream(outFile), BUFFER_SIZE)) {
                        byte[] buf = new byte[BUFFER_SIZE];
                        int len;
                        while ((len = zis.read(buf)) > 0) os.write(buf, 0, len);
                    }
                    zis.closeEntry();
                }
                runOnUiThread(() -> {
                    Toast.makeText(this, R.string.import_success, Toast.LENGTH_SHORT).show();
                    refreshStatus();
                });
            } catch (IOException e) {
                Log.e(TAG, "ZIP import failed", e);
                runOnUiThread(() -> {
                    Toast.makeText(this, getString(R.string.import_failed, e.getMessage()), Toast.LENGTH_LONG).show();
                    refreshStatus();
                });
            } finally {
                runOnUiThread(() -> setWorking(false));
            }
        }).start();
    }

    /**
     * Strips a single wrapper directory from zip entry paths when the entry
     * doesn't start with a known top-level name (e.g. "PvZ/main.pak" -> "main.pak").
     */
    private String stripCommonPrefix(String name) {
        name = name.replace('\\', '/').replaceAll("^/+", "");

        if (isKnownTopLevel(name)) return name;

        // Strip one leading directory component
        int slash = name.indexOf('/');
        if (slash > 0 && slash < name.length() - 1) {
            return name.substring(slash + 1);
        }

        return name;
    }

    private static boolean isKnownTopLevel(String name) {
        return name.startsWith("main.pak") || name.startsWith("properties/") ||
               name.startsWith("Properties/") || name.startsWith("data/") ||
               name.startsWith("images/") || name.startsWith("particles/") ||
               name.startsWith("reanim/") || name.startsWith("sounds/") ||
               name.startsWith("compiled/");
    }

    private File resolveWithinGameDir(File candidate) throws IOException {
        File canonicalGameDir = gameDir.getCanonicalFile();
        File canonicalCandidate = candidate.getCanonicalFile();
        String gameDirPath = canonicalGameDir.getPath();
        String candidatePath = canonicalCandidate.getPath();
        if (!candidatePath.equals(gameDirPath) && !candidatePath.startsWith(gameDirPath + File.separator)) {
            throw new IOException("Import entry is outside the game directory");
        }
        return canonicalCandidate;
    }

    private void importFromDirectory(Uri treeUri) {
        setWorking(true);
        new Thread(() -> {
            try {
                DocumentFile root = DocumentFile.fromTreeUri(this, treeUri);
                if (root == null) throw new IOException("Cannot open directory");

                // If main.pak isn't here, check one level of subdirectories
                DocumentFile sourceDir = root;
                if (root.findFile("main.pak") == null) {
                    for (DocumentFile child : root.listFiles()) {
                        if (child.isDirectory()) {
                            DocumentFile nested = child.findFile("main.pak");
                            if (nested != null) {
                                sourceDir = child;
                                break;
                            }
                        }
                    }
                }

                copyDocumentTree(sourceDir, gameDir);

                runOnUiThread(() -> {
                    Toast.makeText(this, R.string.import_success, Toast.LENGTH_SHORT).show();
                    refreshStatus();
                });
            } catch (IOException e) {
                Log.e(TAG, "Directory import failed", e);
                runOnUiThread(() -> {
                    Toast.makeText(this, getString(R.string.import_failed, e.getMessage()), Toast.LENGTH_LONG).show();
                    refreshStatus();
                });
            } finally {
                runOnUiThread(() -> setWorking(false));
            }
        }).start();
    }

    private void copyDocumentTree(DocumentFile src, File destDir) throws IOException {
        File safeDestDir = resolveWithinGameDir(destDir);
        if (!safeDestDir.exists()) safeDestDir.mkdirs();
        for (DocumentFile child : src.listFiles()) {
            if (child.isDirectory()) {
                String name = child.getName();
                if (name == null) continue;
                copyDocumentTree(child, resolveWithinGameDir(new File(safeDestDir, name)));
            } else {
                String name = child.getName();
                if (name == null) continue;
                File outFile = resolveWithinGameDir(new File(safeDestDir, name));
                try (InputStream is = getContentResolver().openInputStream(child.getUri());
                     OutputStream os = new BufferedOutputStream(new FileOutputStream(outFile), BUFFER_SIZE)) {
                    if (is == null) continue;
                    byte[] buf = new byte[BUFFER_SIZE];
                    int len;
                    while ((len = is.read(buf)) > 0) os.write(buf, 0, len);
                }
            }
        }
    }

    private void exportSaveData(Uri destUri) {
        setWorking(true);
        new Thread(() -> {
            File saveDir = new File(gameDir, "userdata");
            if (!saveDir.isDirectory()) {
                runOnUiThread(() -> {
                    Toast.makeText(this, R.string.no_save_data, Toast.LENGTH_SHORT).show();
                    setWorking(false);
                });
                return;
            }
            try (OutputStream os = getContentResolver().openOutputStream(destUri);
                 java.util.zip.ZipOutputStream zos = new java.util.zip.ZipOutputStream(os)) {
                addDirToZip(zos, saveDir, "userdata/");
                runOnUiThread(() -> {
                    Toast.makeText(this, R.string.export_success, Toast.LENGTH_SHORT).show();
                    refreshStatus();
                });
            } catch (IOException e) {
                Log.e(TAG, "Save export failed", e);
                runOnUiThread(() ->
                    Toast.makeText(this, getString(R.string.export_failed, e.getMessage()), Toast.LENGTH_LONG).show()
                );
            } finally {
                runOnUiThread(() -> setWorking(false));
            }
        }).start();
    }

    private void addDirToZip(java.util.zip.ZipOutputStream zos, File dir, String prefix) throws IOException {
        File[] files = dir.listFiles();
        if (files == null) return;
        for (File f : files) {
            if (f.isDirectory()) {
                addDirToZip(zos, f, prefix + f.getName() + "/");
            } else {
                zos.putNextEntry(new ZipEntry(prefix + f.getName()));
                try (InputStream is = new java.io.FileInputStream(f)) {
                    byte[] buf = new byte[BUFFER_SIZE];
                    int len;
                    while ((len = is.read(buf)) > 0) zos.write(buf, 0, len);
                }
                zos.closeEntry();
            }
        }
    }

    private void importSaveDataFromDir(Uri treeUri) {
        setWorking(true);
        new Thread(() -> {
            try {
                DocumentFile root = DocumentFile.fromTreeUri(this, treeUri);
                if (root == null) throw new IOException("Cannot open directory");

                // Resolve source: pick the userdata/ subfolder if present
                DocumentFile sourceDir = root;
                File destDir = new File(gameDir, "userdata");
                if (!"userdata".equals(root.getName())) {
                    DocumentFile nested = root.findFile("userdata");
                    if (nested != null && nested.isDirectory()) {
                        sourceDir = nested;
                    }
                }

                copyDocumentTree(sourceDir, destDir);

                runOnUiThread(() -> {
                    Toast.makeText(this, R.string.import_success, Toast.LENGTH_SHORT).show();
                    refreshStatus();
                });
            } catch (IOException e) {
                Log.e(TAG, "Save directory import failed", e);
                runOnUiThread(() -> {
                    Toast.makeText(this, getString(R.string.import_failed, e.getMessage()), Toast.LENGTH_LONG).show();
                    refreshStatus();
                });
            } finally {
                runOnUiThread(() -> setWorking(false));
            }
        }).start();
    }

    private void importSaveDataFromZip(Uri uri) {
        setWorking(true);
        new Thread(() -> {
            try (InputStream is = getContentResolver().openInputStream(uri);
                 ZipInputStream zis = new ZipInputStream(is)) {
                ZipEntry entry;
                while ((entry = zis.getNextEntry()) != null) {
                    if (entry.isDirectory()) { zis.closeEntry(); continue; }
                    String name = entry.getName();
                    if (!name.startsWith("userdata/")) name = "userdata/" + name;
                    File outFile = resolveWithinGameDir(new File(gameDir, name));
                    File parent = outFile.getParentFile();
                    if (parent != null && !parent.exists()) parent.mkdirs();
                    try (OutputStream os = new BufferedOutputStream(new FileOutputStream(outFile), BUFFER_SIZE)) {
                        byte[] buf = new byte[BUFFER_SIZE];
                        int len;
                        while ((len = zis.read(buf)) > 0) os.write(buf, 0, len);
                    }
                    zis.closeEntry();
                }
                runOnUiThread(() -> {
                    Toast.makeText(this, R.string.import_success, Toast.LENGTH_SHORT).show();
                    refreshStatus();
                });
            } catch (IOException e) {
                Log.e(TAG, "Save import failed", e);
                runOnUiThread(() ->
                    Toast.makeText(this, getString(R.string.import_failed, e.getMessage()), Toast.LENGTH_LONG).show()
                );
            } finally {
                runOnUiThread(() -> setWorking(false));
            }
        }).start();
    }

    private void setWorking(boolean working) {
        progressBar.setVisibility(working ? View.VISIBLE : View.GONE);
        btnPickZip.setEnabled(!working);
        btnPickDir.setEnabled(!working);
        btnExportSave.setEnabled(!working && hasSaveData());
        btnImportSaveZip.setEnabled(!working);
        btnImportSaveDir.setEnabled(!working);
        btnLaunchGame.setEnabled(!working && hasResources());
    }
}

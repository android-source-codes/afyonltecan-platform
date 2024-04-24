// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser;

import android.content.Context;
import android.os.AsyncTask;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import javax.crypto.KeyGenerator;
import javax.crypto.Mac;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;

/**
 * Authenticate the source of Intents to launch web apps (see e.g. {@link #FullScreenActivity}).
 *
 * Chrome does not keep a store of valid URLs for installed web apps (because it cannot know when
 * any have been uninstalled). Therefore, upon installation, it tells the Launcher a message
 * authentication code (MAC) along with the URL for the web app, and then Chrome can verify the MAC
 * when starting e.g. {@link #FullScreenActivity}. Chrome can thus distinguish between legitimate,
 * installed web apps and arbitrary other URLs.
 */
public class WebappAuthenticator {
    private static final String TAG = "WebappAuthenticator";
    private static final String MAC_ALGORITHM_NAME = "HmacSHA256";
    private static final String MAC_KEY_BASENAME = "webapp-authenticator";
    private static final int MAC_KEY_BYTE_COUNT = 32;
    private static final Object sLock = new Object();

    private static FutureTask<SecretKey> sMacKeyGenerator;
    private static SecretKey sKey = null;

    /**
     * @see #getMacForUrl
     *
     * @param url The URL to validate.
     * @param mac The bytes of a previously-calculated MAC.
     *
     * @return true if the MAC is a valid MAC for the URL, false otherwise.
     */
    public static boolean isUrlValid(Context context, String url, byte[] mac) {
        byte[] goodMac = getMacForUrl(context, url);
        if (goodMac == null) {
            return false;
        }
        return constantTimeAreArraysEqual(goodMac, mac);
    }

    /**
     * @see #isUrlValid
     *
     * @param url A URL for which to calculate a MAC.
     *
     * @return The bytes of a MAC for the URL, or null if a secure MAC was not available.
     */
    public static byte[] getMacForUrl(Context context, String url) {
        Mac mac = getMac(context);
        if (mac == null) {
            return null;
        }
        return mac.doFinal(url.getBytes());
    }

    // TODO(palmer): Put this method, and as much of this class as possible, in a utility class.
    private static boolean constantTimeAreArraysEqual(byte[] a, byte[] b) {
        if (a.length != b.length) {
            return false;
        }

        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }

    private static SecretKey readKeyFromFile(
            Context context, String basename, String algorithmName) {
        FileInputStream input = null;
        File file = context.getFileStreamPath(basename);
        try {
            if (file.length() != MAC_KEY_BYTE_COUNT) {
                Log.w(TAG, "Could not read key from '" + file + "': invalid file contents");
                return null;
            }

            byte[] keyBytes = new byte[MAC_KEY_BYTE_COUNT];
            input = new FileInputStream(file);
            if (MAC_KEY_BYTE_COUNT != input.read(keyBytes)) {
                return null;
            }
            return new SecretKeySpec(keyBytes, algorithmName);
           
        }catch (FileNotFoundException e1	) {
       	    Log.w(TAG, "Could not read key from '" + file + "': " + e1.getMessage());
            return null;
	}  catch (IllegalArgumentException e2) {
            Log.w(TAG, "Could not read key from '" + file + "': " + e2.getMessage());
            return null;
        } catch (IOException e3) {
	    Log.w(TAG, "Could not read key from '" + file + "': " + e3.getMessage());
            return null;
	} finally {
            try {
                if (input != null) {
                    input.close();
                }
            } catch (IOException e4) {
                Log.e(TAG, "Could not close key input stream '" + file + "': " + e4.getMessage());
            }
        }
    }

    private static boolean writeKeyToFile(Context context, String basename, SecretKey key) {
        File file = context.getFileStreamPath(basename);
        byte[] keyBytes = key.getEncoded();
        if (MAC_KEY_BYTE_COUNT != keyBytes.length) {
            Log.e(TAG, "writeKeyToFile got key encoded bytes length " + keyBytes.length +
                       "; expected " + MAC_KEY_BYTE_COUNT);
            return false;
        }
		FileOutputStream output = null;
        try {
            output = new FileOutputStream(file);
            output.write(keyBytes);
            return true;
        } catch (FileNotFoundException e1) {
            Log.e(TAG, "Could not write key to '" + file + "': " + e1.getMessage());
            return false;
        } catch (IOException e2) {
            Log.e(TAG, "Could not write key to '" + file + "': " + e2.getMessage());
            return false;
        }
        finally{
            if(output != null){
                try {
                    output.close();
                } catch (IOException e) {
                    Log.e(TAG, "Could not close key output stream '" + file + "': " + e.getMessage());
                }
            }
        }
    }

    private static SecretKey getKey(Context context) {
        synchronized (sLock) {
            if (sKey == null) {
                SecretKey key = readKeyFromFile(context, MAC_KEY_BASENAME, MAC_ALGORITHM_NAME);
                if (key != null) {
                    sKey = key;
                    return sKey;
                }

                triggerMacKeyGeneration();
                try {
                    sKey = sMacKeyGenerator.get();
                    sMacKeyGenerator = null;
                    if (!writeKeyToFile(context, MAC_KEY_BASENAME, sKey)) {
                        sKey = null;
                        return null;
                    }
                    return sKey;
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                } catch (ExecutionException e) {
                    throw new RuntimeException(e);
                }
            }
            return sKey;
        }
    }

    /**
     * Generates the authentication encryption key in a background thread (if necessary).
     */
    private static void triggerMacKeyGeneration() {
        synchronized (sLock) {
            if (sKey != null || sMacKeyGenerator != null) {
                return;
            }

            sMacKeyGenerator = new FutureTask<SecretKey>(new Callable<SecretKey>() {
                @Override
                public SecretKey call() throws Exception {
                    KeyGenerator generator = KeyGenerator.getInstance(MAC_ALGORITHM_NAME);
                    SecureRandom random = SecureRandom.getInstance("SHA1PRNG");

                    // Versions of SecureRandom from Android <= 4.3 do not seed themselves as
                    // securely as possible. This workaround should suffice until the fixed version
                    // is deployed to all users. getRandomBytes, which reads from /dev/urandom,
                    // which is as good as the platform can get.
                    //
                    // TODO(palmer): Consider getting rid of this once the updated platform has
                    // shipped to everyone. Alternately, leave this in as a defense against other
                    // bugs in SecureRandom.
                    byte[] seed = getRandomBytes(MAC_KEY_BYTE_COUNT);
                    if (seed == null) {
                        return null;
                    }
                    random.setSeed(seed);
                    generator.init(MAC_KEY_BYTE_COUNT * 8, random);
                    return generator.generateKey();
                }
            });
            AsyncTask.THREAD_POOL_EXECUTOR.execute(sMacKeyGenerator);
        }
    }

    private static byte[] getRandomBytes(int count) {
        FileInputStream fis = null;
        try {
            fis = new FileInputStream("/dev/urandom");
            byte[] bytes = new byte[count];
            if (bytes.length != fis.read(bytes)) {
                return null;
            }
            return bytes;
        } catch (Throwable t) {
            // This causes the ultimate caller, i.e. getMac, to fail.
            return null;
        } finally {
            try {
                if (fis != null) {
                    fis.close();
                }
            } catch (IOException e) {
                // Nothing we can do.
            }
        }
    }

    /**
     * @return A Mac, or null if it is not possible to instantiate one.
     */
    private static Mac getMac(Context context) {
        try {
            SecretKey key = getKey(context);
            if (key == null) {
                // getKey should have invoked triggerMacKeyGeneration, which should have set the
                // random seed and generated a key from it. If not, there is a problem with the
                // random number generator, and we must not claim that authentication can work.
                return null;
            }
            Mac mac = Mac.getInstance(MAC_ALGORITHM_NAME);
            mac.init(key);
            return mac;
        } catch (GeneralSecurityException e) {
            Log.w(TAG, "Error in creating MAC instance", e);
            return null;
        }
    }
}

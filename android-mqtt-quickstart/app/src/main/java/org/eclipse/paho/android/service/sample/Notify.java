/*******************************************************************************
 * Copyright (c) 1999, 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution. 
 *
 * The Eclipse Public License is available at 
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 */
package org.eclipse.paho.android.service.sample;

import java.security.SecureRandom;
import java.util.Calendar;
import org.eclipse.paho.android.service.sample.R;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.support.v4.app.NotificationCompat.Builder;
import android.widget.Toast;

import java.security.SecureRandom;
import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;


/**
 * Provides static methods for creating and showing notifications to the user.
 *
 */
public class Notify {

  /* temp = Encrypted or plain text data
  *   toEncrypt = true = encryption
  *   toEncrypt = false = decryption*/
  public static byte[] encdec(byte[] temp, boolean toEncrypt)
  {
    byte[] rawkey = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};//"0102030405060708090A0B0C0D0E0F".getBytes();
    byte[] finalString = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    try{
      if(true == toEncrypt)
      {
        finalString =  encrypt(rawkey , temp);

      }else
      {
        finalString = decrypt(rawkey , temp);

      }
    }
    catch (Exception e) {
      e.printStackTrace();
    }
    return finalString;
  }
/*
  encrypt with AES - No Padding
 */

  private static byte[] encrypt(byte[] raw, byte[] clear) throws Exception {
    SecretKeySpec skeySpec = new SecretKeySpec(raw, "AES");
    Cipher cipher = Cipher.getInstance("AES/ECB/NoPadding");
    cipher.init(Cipher.ENCRYPT_MODE, skeySpec);
    byte[] encrypted = cipher.doFinal(clear);
    return encrypted;
  }

  private static byte[] decrypt(byte[] raw, byte[] encrypted) throws Exception {
    SecretKeySpec skeySpec = new SecretKeySpec(raw, "AES");
    Cipher cipher = Cipher.getInstance("AES/ECB/NoPadding");
    cipher.init(Cipher.DECRYPT_MODE, skeySpec);
    byte[] decrypted = cipher.doFinal(encrypted);
    return decrypted;
  }


  /*
  public static String encrypt(byte[] rawKey , String cleartext) throws Exception {
    //byte[] rawKey = getRawKey(seed.getBytes());
    byte[] result = encrypt(rawKey, cleartext.getBytes());
    return toHex(result);
  }

  public static String decrypt(byte[] rawKey , String encrypted) throws Exception {
    //byte[] rawKey = getRawKey(seed.getBytes());
    byte[] enc = toByte(encrypted);
    byte[] result = decrypt(rawKey, enc);
    return new String(result);
  }

  private static byte[] getRawKey(byte[] seed) throws Exception {
    KeyGenerator kgen = KeyGenerator.getInstance("AES");
    SecureRandom sr = SecureRandom.getInstance("SHA1PRNG","Crypto");
    sr.setSeed(seed);
    kgen.init(128, sr); // 192 and 256 bits may not be available
    SecretKey skey = kgen.generateKey();
    byte[] raw = skey.getEncoded();
    return raw;
  }

*/

  public static String toHex(String txt) {
    return toHex(txt.getBytes());
  }
  public static String fromHex(String hex) {
    return new String(toByte(hex));
  }

  public static byte[] toByte(String hexString) {
    int len = hexString.length()/2;
    byte[] result = new byte[len];
    for (int i = 0; i < len; i++)
      result[i] = Integer.valueOf(hexString.substring(2*i, 2*i+2), 16).byteValue();
    return result;
  }

  public static String toHex(byte[] buf) {
    if (buf == null)
      return "";
    StringBuffer result = new StringBuffer(2*buf.length);
    for (int i = 0; i < buf.length; i++) {
      appendHex(result, buf[i]);
    }
    return result.toString();
  }
  private final static String HEX = "0123456789ABCDEF";
  private static void appendHex(StringBuffer sb, byte b) {
    sb.append(HEX.charAt((b>>4)&0x0f)).append(HEX.charAt(b&0x0f));
  }

  /** Message ID Counter **/
  private static int MessageID = 0;

  /**
   * Displays a notification in the notification area of the UI
   * @param context Context from which to create the notification
   * @param messageString The string to display to the user as a message
   * @param intent The intent which will start the activity when the user clicks the notification
   * @param notificationTitle The resource reference to the notification title
   */
  static void notifcation(Context context, String messageString, Intent intent, int notificationTitle) {

    //Get the notification manage which we will use to display the notification
    String ns = Context.NOTIFICATION_SERVICE;
    NotificationManager mNotificationManager = (NotificationManager) context.getSystemService(ns);

    Calendar.getInstance().getTime().toString();

    long when = System.currentTimeMillis();

    //get the notification title from the application's strings.xml file
    CharSequence contentTitle = context.getString(notificationTitle);


    String encryptedString = "";//encdec(messageString,true);
    String decryptedString = ""; //encdec(encryptedString,false);

    String display = "\n"+ messageString + "\n" + encryptedString + "\n" + decryptedString;

    //the message that will be displayed as the ticker
    String ticker = contentTitle + " " + display;



    //build the pending intent that will start the appropriate activity
    PendingIntent pendingIntent = PendingIntent.getActivity(context,
        ActivityConstants.showHistory, intent, 0);

    //build the notification
    Builder notificationCompat = new Builder(context);
    notificationCompat.setAutoCancel(true)
        .setContentTitle(contentTitle)
        .setContentIntent(pendingIntent)
        .setContentText(display)
        .setTicker(ticker)
        .setWhen(when)
        .setSmallIcon(R.drawable.ic_launcher);

    Notification notification = notificationCompat.build();
    //display the notification
    mNotificationManager.notify(MessageID, notification);
    MessageID++;

  }

  /**
   * Display a toast notification to the user
   * @param context Context from which to create a notification
   * @param text The text the toast should display
   * @param duration The amount of time for the toast to appear to the user
   */
  static void toast(Context context, CharSequence text, int duration) {
    Toast toast = Toast.makeText(context, text, duration);
    toast.show();
  }

}

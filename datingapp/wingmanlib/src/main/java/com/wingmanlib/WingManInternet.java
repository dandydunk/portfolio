package com.wingmanlib;
import android.util.Log;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URL;
import java.net.HttpURLConnection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.io.OutputStream;
import java.nio.charset.Charset;

public class WingManInternet {
    public static String SendHTTPPost(String url, HashMap<String, String> mapParams) throws java.io.IOException {
        Iterator it = mapParams.entrySet().iterator();
        StringBuilder sb = new StringBuilder();
        int sizeMapParams = mapParams.size();
        int loopC = 0;
        while (it.hasNext()) {
            Map.Entry pair = (Map.Entry)it.next();
            sb.append(pair.getKey());
            sb.append("=");
            sb.append(pair.getValue());

            if(++loopC < sizeMapParams) {
                sb.append("&");
            }
            it.remove();
        }

        //Log.d("trace", "posting ["+sb.toString()+"] to ["+url+"]");
        URL u = new URL(url);
        HttpURLConnection conn = (HttpURLConnection)u.openConnection();
        conn.setRequestMethod("POST");
        conn.setDoOutput(true);
        OutputStream os = conn.getOutputStream();
        os.write(sb.toString().getBytes(Charset.forName("UTF-8")));
        os.flush();
        os.close();

        BufferedReader br = new BufferedReader(new InputStreamReader(conn.getInputStream()));
        StringBuilder sbHttpResponse = new StringBuilder();
        do {
            String line = br.readLine();
            if(null == line) break;
            sbHttpResponse.append(line);
        } while(true);

        br.close();
        return sbHttpResponse.toString();
    }

    public static String SendHTTPGet(String url) throws java.io.IOException {
        URL u = new URL(url);
        HttpURLConnection conn = (HttpURLConnection)u.openConnection();
        conn.setRequestMethod("GET");

        BufferedReader br = new BufferedReader(new InputStreamReader(conn.getInputStream()));
        StringBuilder sbHttpResponse = new StringBuilder();
        do {
            String line = br.readLine();
            if(null == line) break;
            sbHttpResponse.append(line);
        } while(true);

        br.close();
        return sbHttpResponse.toString();
    }
}

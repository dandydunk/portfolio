package com.wingmanlib;

import android.util.Base64;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONTokener;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;

import static android.util.Base64.DEFAULT;

/**
 * Created by deymack on 2/4/2017.
 */

public class WingManWebServiceInbox extends WingManWebService {
    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    int id;

    public String getUserSessionId() {
        return userSessionId;
    }
    public void setUserSessionId(String userSessionId) {
        this.userSessionId = userSessionId;
    }

    String userSessionId;

    public int getStage() {
        return stage;
    }

    public void setStage(int stage) {
        this.stage = stage;
    }

    int stage;

    public int getStartUserId() {
        return startUserId;
    }

    public void setStartUserId(int startUserId) {
        this.startUserId = startUserId;
    }

    int startUserId;

    public void NewInbox(int fromUserId) throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("toUserId", Integer.toString(fromUserId));
        mapParams.put("sessionId", this.userSessionId);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("newInbox"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }

        if(jo.has("inboxId")) {
            this.id = jo.getInt("inboxId");
        }
    }

    public void GetInbox(int userId) throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("toUserId", Integer.toString(userId));
        mapParams.put("sessionId", this.userSessionId);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("getInbox"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            if(jo.has("errorNumber")) {
                int errNum = jo.getInt("errorNumber");
                if (errNum == 100) {
                    return;
                }
            }
            throw new WingManException(jo.getString("error"));
        }

        this.startUserId = jo.getInt("startUserId");
        this.stage = jo.getInt("stage");
        this.id = jo.getInt("inboxId");
    }

    public int GetInboxOtherUser() throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("sessionId", this.userSessionId);
        mapParams.put("inboxId", Integer.toString(this.id));

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("getInboxOtherUser"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }

        return jo.getInt("userId");
    }

    public void NewMessage(String msg) throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("inboxId", Integer.toString(this.id));
        mapParams.put("sessionId", this.userSessionId);
        mapParams.put("chatMessage", msg);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("newMessage"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }
    }

    public void Approve(int inboxId) throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("inboxId", Integer.toString(inboxId));
        mapParams.put("sessionId", this.userSessionId);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("approveInbox"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }
    }

    public void Cancel(int inboxId) throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("inboxId", Integer.toString(inboxId));
        mapParams.put("sessionId", this.userSessionId);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("cancelInbox"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }
    }

    public void Reject(int inboxId) throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("inboxId", Integer.toString(inboxId));
        mapParams.put("sessionId", this.userSessionId);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("rejectInbox"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }
    }

    public void GetAuthorizations() throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("sessionId", this.userSessionId);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("inboxAuthorizations"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }
    }

    public ArrayList<WingManInboxMessage> GetNewMessages() throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("sessionId", this.userSessionId);
        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("getNewInboxMessages"), mapParams);
        //Log.d("info", "got response["+httpResponse+"]");
        Object json = new JSONTokener(httpResponse).nextValue();
        ArrayList<WingManInboxMessage> listWu = new ArrayList<WingManInboxMessage>();
        if (json instanceof JSONObject) {
            JSONObject jo = new JSONObject(httpResponse);
            if (jo.has("error")) {
                int errNum = jo.getInt("errorNumber");
                if(errNum == 100) {
                    return listWu;
                }
                throw new WingManException(jo.getString("error"));
            }
        }

        else if (json instanceof JSONArray) {
            JSONArray ja = new JSONArray(httpResponse);
            int jaLen = ja.length();

            for (int i = 0; i < jaLen; i++) {
                JSONObject jo = ja.getJSONObject(i);
                WingManInboxMessage imp = new WingManInboxMessage();
                int userId = jo.getInt("userId");

                WingManWebServiceUser wUser = new WingManWebServiceUser();
                wUser.setUserId(userId);
                wUser.GetProfileById();


                imp.fromUser = wUser;

                WingManWebServiceUser rUser = new WingManWebServiceUser();
                rUser.setUserId(jo.getInt("recipientUserId"));
                rUser.GetProfileById();

                imp.recipientUser = rUser;
                imp.message = jo.getString("message");
                imp.dateSent = jo.getString("dateSent");
                imp.id = jo.getInt("messageId");
                imp.inboxId = jo.getInt("inboxId");
                imp.fromUserId = userId;
                listWu.add(imp);
            }
        }

        return listWu;
    }

    public ArrayList<WingManInbox> GetRequests() throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("sessionId", this.userSessionId);
        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("newRequests"), mapParams);
        //Log.d("info", "got response["+httpResponse+"]");
        Object json = new JSONTokener(httpResponse).nextValue();
        ArrayList<WingManInbox> listWu = new ArrayList<WingManInbox>();
        if (json instanceof JSONObject) {
            JSONObject jo = new JSONObject(httpResponse);
            if (jo.has("error")) {
                int errNum = jo.getInt("errorNumber");
                if(errNum == 101) {
                    return listWu;
                }
                throw new WingManException(jo.getString("error"));
            }
        }

        else if (json instanceof JSONArray) {
            JSONArray ja = new JSONArray(httpResponse);
            int jaLen = ja.length();

            for (int i = 0; i < jaLen; i++) {
                JSONObject jo = ja.getJSONObject(i);
                WingManInbox imp = new WingManInbox();
                int userId = jo.getInt("startUserId");

                WingManWebServiceUser wUser = new WingManWebServiceUser();
                wUser.setUserId(userId);
                wUser.GetProfileById();

                imp.fromUser = wUser;

                imp.dateSent = jo.getString("timeCreated");
                imp.id = jo.getInt("id");
                listWu.add(imp);
            }
        }

        return listWu;
    }
}

package com.wingmand;

import android.content.Context;
import android.location.Location;
import android.location.LocationManager;
import android.os.Looper;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.PendingResult;
import com.google.android.gms.common.api.ResultCallback;
import com.google.android.gms.common.api.Status;
import com.google.android.gms.location.LocationListener;
import com.google.android.gms.location.LocationRequest;
import com.google.android.gms.location.LocationServices;
import com.google.android.gms.location.LocationSettingsRequest;
import com.google.android.gms.location.LocationSettingsResult;
import com.google.android.gms.location.LocationSettingsStates;
import com.google.android.gms.location.LocationSettingsStatusCodes;
import com.wingmanlib.WingManUtils;

public class GPSActivity extends AppCompatActivity implements
        GoogleApiClient.ConnectionCallbacks, GoogleApiClient.OnConnectionFailedListener, LocationListener {
    GoogleApiClient googleClient = null;
    LocationRequest mLocationRequest;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d("trace", "google gps Connection created");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_gps);

        if(null == googleClient) {
            googleClient = new GoogleApiClient.Builder(this)
                    .addConnectionCallbacks(this)
                    .addOnConnectionFailedListener(this)
                    .addApi(LocationServices.API)
                    .build();
        }
    }

    @Override
    protected void onStart() {
        Log.d("trace", "google gps Connection started");

        googleClient.connect();
        super.onStart();
    }

    @Override
    protected void onResume() {
        Log.d("trace", "google gps Connection resumed");
        super.onResume();
        if(googleClient != null) {
            googleClient.connect();
        }
        //super.onStart();
        //googleClient.connect();
    }

    @Override
    protected void onStop() {
        Log.d("trace", "google gps Connection stopped");


        if (googleClient.isConnected()) {
            googleClient.disconnect();
        }
        super.onStop();
    }

    @Override
    public void onConnected(Bundle connectionHint) {
        Log.d("trace", "google gps CONNECTING...");
        mLocationRequest = LocationRequest.create();
        mLocationRequest.setInterval(5000);
        mLocationRequest.setPriority(LocationRequest.PRIORITY_HIGH_ACCURACY);

        try {
            LocationServices.FusedLocationApi.requestLocationUpdates(googleClient, mLocationRequest, this);
        } catch(SecurityException ex) {
            Log.d("info", "google security exception", ex);
            return;
        }
        Log.d("trace", "google gps CONNECTED...");
    }

    @Override
    public void onLocationChanged(Location location) {
        Log.d("trace", "google gps location changed");
        SessionManager.GPS.lastLocation = location;
        double latitude = SessionManager.GPS.lastLocation.getLatitude();
        double longitude = SessionManager.GPS.lastLocation.getLongitude();
        Log.d("info", String.format("lat [%f] long [%f]", latitude, longitude));
        finish();
        onBackPressed();
    }

    protected void createLocationRequest() {

    }

    @Override
    public void onConnectionFailed(ConnectionResult result) {
        Log.d("trace", "Google location Connection failed: ConnectionResult.getErrorCode() = " + result.getErrorCode());
        WingManUtils.ShowErrorDialog(this, "There was an error getting your location. Please try again later.");
    }


    @Override
    public void onConnectionSuspended(int cause) {
        Log.d("trace", "Connection suspended");
        googleClient.connect();
    }
}

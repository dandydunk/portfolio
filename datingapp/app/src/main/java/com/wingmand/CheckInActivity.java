package com.wingmand;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.location.Location;
import android.os.Looper;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.location.LocationListener;
import com.google.android.gms.location.LocationRequest;
import com.google.android.gms.location.LocationServices;
import com.google.android.gms.location.places.Place;
import com.google.android.gms.location.places.ui.PlacePicker;
import com.google.android.gms.maps.CameraUpdateFactory;
import com.google.android.gms.maps.GoogleMap;
import com.google.android.gms.maps.OnMapReadyCallback;
import com.google.android.gms.maps.SupportMapFragment;
import com.google.android.gms.maps.model.LatLng;
import com.google.android.gms.maps.model.MarkerOptions;
import com.wingmanlib.WingManException;
import com.wingmanlib.WingManLocation;
import com.wingmanlib.WingManSettings;
import com.wingmanlib.WingManUtils;
import com.wingmanlib.WingManWebServiceUser;

import java.io.IOException;


public class CheckInActivity extends AppCompatActivity implements OnMapReadyCallback,
        GoogleApiClient.ConnectionCallbacks, GoogleApiClient.OnConnectionFailedListener, LocationListener {
    GoogleApiClient googleClient = null;
    LocationRequest mLocationRequest;
    LatLng lastLatLng = null;
    GoogleMap googleMap = null;
    int PLACE_PICKER_REQUEST = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_check_in);

        Toolbar myToolbar = (Toolbar) findViewById(R.id.my_toolbar);
        setSupportActionBar(myToolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        SupportMapFragment mapFragment = (SupportMapFragment) getSupportFragmentManager().findFragmentById(R.id.map);
        mapFragment.getMapAsync(this);
    }

    @Override
    protected void onStart() {
        super.onStart();

        UpdateLocation();
    }

    void UpdateLocation() {
        final Activity context = this;
        final ProgressDialog pd = ProgressDialog.show(this, "Loading", "Checking if you have checked in...");
        final Button buttonCheckIn = (Button)findViewById(R.id.buttonCheckIn);
        buttonCheckIn.setEnabled(false);

        new Thread(new Runnable() {
            @Override
            public void run() {
                android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);

                WingManWebServiceUser user = new WingManWebServiceUser();
                user.setSessionId(SessionManager.getSessionId());
                WingManLocation wLocation = null;
                try {
                    Log.d("info", "getting location...");
                    wLocation = user.GetLocation();
                } catch(final Exception ex) {
                    Log.d("stackTrace", "exception", ex);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            WingManUtils.ShowErrorDialog(context, ex.getMessage());
                            pd.dismiss();
                            buttonCheckIn.setEnabled(true);
                        }
                    });
                    return;
                }

                final WingManLocation wLocation_ = wLocation;

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if(null != wLocation_) {
                            TellPlace(wLocation_);
                            SessionManager.GPS.lastLocation = new Location("loc");
                            SessionManager.GPS.lastLocation.setLongitude(wLocation_.longitude);
                            SessionManager.GPS.lastLocation.setLatitude(wLocation_.latitude);
                            updateMapLocation();
                        }
                        buttonCheckIn.setEnabled(true);
                        pd.dismiss();
                    }
                });
            }
        }).start();
    }

    @Override
    protected void onStop() {
        Log.d("trace", "stopping check in activity");

        if (googleClient != null && googleClient.isConnected()) {
            googleClient.disconnect();
        }

        super.onStop();
    }

    @Override
    public void onConnected(Bundle connectionHint) {
        Log.d("trace", "google gps connected...");
        Location mLastLocation = null;
        try {
            mLastLocation = LocationServices.FusedLocationApi.getLastLocation(googleClient);
        } catch(SecurityException ex) {
            Log.d("info", "google security exception", ex);
            WingManUtils.ShowErrorDialog(this, "There was a security error getting your location.");
            googleClient.disconnect();
            Button button = (Button) findViewById(R.id.buttonCheckIn);
            button.setEnabled(true);

            return;
        }

        if(mLastLocation != null) {
            onLocationChanged(mLastLocation);
            return;
        }

        mLocationRequest = LocationRequest.create();
        mLocationRequest.setInterval(5000);
        mLocationRequest.setPriority(LocationRequest.PRIORITY_HIGH_ACCURACY);

        try {
            LocationServices.FusedLocationApi.requestLocationUpdates(googleClient, mLocationRequest, this);
        } catch(SecurityException ex) {
            Log.d("info", "google security exception", ex);
            WingManUtils.ShowErrorDialog(this, "There was a security error getting your location.");
            googleClient.disconnect();
            Button button = (Button) findViewById(R.id.buttonCheckIn);
            button.setEnabled(true);
            return;
        }

        Log.d("trace", "google gps CONNECTED...");
    }

    @Override
    public void onLocationChanged(final Location location) {
        Log.d("trace", "google gps location changed");
        SessionManager.GPS.lastLocation = location;

        googleClient.disconnect();

        Log.d("trace", "updating the map location...");
        this.updateMapLocation();

        final CheckInActivity naa = this;
        final Button button = (Button) findViewById(R.id.buttonCheckIn);
        Thread thread = new Thread(new Runnable(){
            @Override
            public void run(){
                Looper.prepare();

                WingManWebServiceUser user = new WingManWebServiceUser();
                user.setSessionId(SessionManager.getSessionId());
                try {
                    Log.d("info", "checking in...");
                    //user.CheckIn(location);
                } catch(final Exception ex) {
                    Log.d("stackTrace", "exception", ex);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            WingManUtils.ShowErrorDialog(naa, ex.getMessage());
                            button.setEnabled(true);
                        }
                    });
                    return;
                }
                Log.d("info", "checked in.");
                runOnUiThread(new Runnable() {
                    public void run() {
                        button.setEnabled(true);
                    }
                });

            }
        });
        thread.start();


    }

    @Override
    public void onConnectionFailed(ConnectionResult result) {
        Log.d("trace", "Google location Connection failed: ConnectionResult.getErrorCode() = " + result.getErrorCode());
        WingManUtils.ShowErrorDialog(this, "There was an error getting your location. Please try again later.");
    }


    @Override
    public void onConnectionSuspended(int cause) {
        Log.d("trace", "Connection suspended");
        WingManUtils.ShowErrorDialog(this, "The connection was suspended. Please try again later.");
        Button button = (Button) findViewById(R.id.buttonCheckIn);
        button.setEnabled(true);
    }

    @Override
    public void onMapReady(GoogleMap map) {
        Log.d("stacktrace", "map ready to use");
        googleMap = map;

        double latitude = 38.242318;
        double longitude = -98.001561;

        map.addMarker(new MarkerOptions().position(new LatLng(latitude, longitude)).title("Marker"));
        map.moveCamera(CameraUpdateFactory.newLatLng(new LatLng(latitude, longitude)));
        map.moveCamera(CameraUpdateFactory.zoomTo(10));
    }

    public void updateMapLocation() {
        if(null == SessionManager.GPS.lastLocation) {
            return;
        }

        if(googleMap == null) {
            return;
        }

        if(lastLatLng != null) {
            googleMap.clear();
        }

        double latitude = SessionManager.GPS.lastLocation.getLatitude();
        double longitude = SessionManager.GPS.lastLocation.getLongitude();
        Log.d("info", String.format("updating map to: lat [%f] long [%f]", latitude, longitude));

        lastLatLng = new LatLng(SessionManager.GPS.lastLocation.getLatitude(), SessionManager.GPS.lastLocation.getLongitude());
        googleMap.addMarker(new MarkerOptions().position(lastLatLng).title("Marker"));
        googleMap.moveCamera(CameraUpdateFactory.newLatLng(lastLatLng));
        googleMap.moveCamera(CameraUpdateFactory.zoomTo(10));
    }

    void checkIn() {

    }

    public void buttonCheckInClick(View view) throws Exception {
        Button button = (Button) findViewById(R.id.buttonCheckIn);
        button.setEnabled(false);


        PlacePicker.IntentBuilder builder = new PlacePicker.IntentBuilder();
        startActivityForResult(builder.build(this), PLACE_PICKER_REQUEST);

        /*
        if(null == googleClient) {
            googleClient = new
                    GoogleApiClient.Builder(this)
                    .addConnectionCallbacks(this)
                    .addOnConnectionFailedListener(this)
                    .addApi(LocationServices.API)
                    .build();
        }

        if (googleClient.isConnected()) {
            googleClient.disconnect();
        }

        Log.d("stacktrace", "connecting to google location services..");
        googleClient.connect();*/
    }

    void TellPlace(WingManLocation loc) {
        TextView locationAddress = (TextView)findViewById(R.id.textViewLocationAddress);
        TextView locationName = (TextView)findViewById(R.id.textViewLocationName);
        LinearLayout ll = (LinearLayout)findViewById(R.id.linearLayoutLocation);

        locationName.setText(loc.place);
        locationAddress.setText(String.format("%s %s, %s %s %s", loc.street, loc.address, loc.city, loc.state, loc.postalCode));
        ll.setVisibility(View.VISIBLE);
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == PLACE_PICKER_REQUEST) {
            if (resultCode == RESULT_OK) {
                final ProgressDialog pd = ProgressDialog.show(this, "Loading...", "Updating your location...");
                final Activity context = this;

                final Place place = PlacePicker.getPlace(this, data);
                final Location location = new Location("loc");
                location.setLatitude(place.getLatLng().latitude);
                location.setLongitude(place.getLatLng().longitude);

                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);

                        WingManWebServiceUser user = new WingManWebServiceUser();
                        user.setSessionId(SessionManager.getSessionId());
                        try {
                            Log.d("info", "checking in...");
                            user.CheckIn(location, place.getName().toString());
                        } catch(final Exception ex) {
                            Log.d("stackTrace", "exception", ex);
                            runOnUiThread(new Runnable() {
                                public void run() {
                                    WingManUtils.ShowErrorDialog(context, ex.getMessage());
                                    pd.dismiss();
                                }
                            });
                            return;
                        }

                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                pd.dismiss();
                                UpdateLocation();
                            }
                        });
                    }
                }).start();
            }
        }
    }
}

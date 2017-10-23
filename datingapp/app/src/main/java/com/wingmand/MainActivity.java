package com.wingmand;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.support.design.widget.TabLayout;
import android.support.v4.view.ViewPager;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.util.Log;

import com.wingmand.inboxfragments.ThreadGetNewMessagesFromWebService;
import com.wingmand.mainactivityfragments.MainActivityRootTabPagerAdapter;
import com.wingmanlib.WingManSettings;

public class MainActivity extends AppCompatActivity {


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d("trace", "main act created");

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Toolbar myToolbar = (Toolbar) findViewById(R.id.my_toolbar);
        setSupportActionBar(myToolbar);

        TabLayout tabLayout = (TabLayout) findViewById(R.id.tab_layout);
        tabLayout.addTab(tabLayout.newTab().setText("Your Matches"));
        tabLayout.setTabGravity(TabLayout.GRAVITY_FILL);

        final ViewPager viewPager = (ViewPager) findViewById(R.id.pager);
        final MainActivityRootTabPagerAdapter adapter = new MainActivityRootTabPagerAdapter(getSupportFragmentManager(), tabLayout.getTabCount());
        viewPager.setAdapter(adapter);
        viewPager.addOnPageChangeListener(new TabLayout.TabLayoutOnPageChangeListener(tabLayout));
        tabLayout.setOnTabSelectedListener(new TabLayout.OnTabSelectedListener() {
            @Override
            public void onTabSelected(TabLayout.Tab tab) {
                viewPager.setCurrentItem(tab.getPosition());
            }

            @Override
            public void onTabUnselected(TabLayout.Tab tab) {}

            @Override
            public void onTabReselected(TabLayout.Tab tab) {}
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == 1) {
           Log.d("trace", "get gps done");
        }
    }

    @Override
    public void onResume() {
        super.onResume();

        Log.d("trace", "main act resumed");
        if(!SessionManager.appStarted) {
            WingManSettings settings = new WingManSettings(this);
            SessionManager.setSessionId(settings.GetByName("sessionId"));
            String userIdS = settings.GetByName("userId");
            if(userIdS != null && userIdS.length() > 0) {
                int userId = Integer.parseInt(userIdS);
                SessionManager.setUserId(userId);
            }

            SessionManager.appStarted = true;
        }

        if(null == SessionManager.getSessionId() || 0 == SessionManager.getSessionId().length()) {
            Intent i = new Intent(this, LoginActivity.class);
            startActivity(i);
            return;
        }
    }

    @Override
    public void onPause() {
        super.onPause();  // Always call the superclass method first

        Log.d("trace", "main act paused");
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.main_menu, menu);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        if(!SessionManager.isLoggedIn()) {
            menu.setGroupVisible(R.id.groupMainMenuIsLoggedIn, false);
            menu.setGroupVisible(R.id.groupMainMenuIsLoggedOut, true);
        } else {
            menu.setGroupVisible(R.id.groupMainMenuIsLoggedIn, true);
            menu.setGroupVisible(R.id.groupMainMenuIsLoggedOut, false);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        Intent intent = null;
        switch (item.getItemId()) {
            case R.id.loginMenuItem:
                intent = new Intent(this, LoginActivity.class);
            break;

            case R.id.registerMenuItem:
               intent = new Intent(this, NewAccountActivity.class);
            break;

            case R.id.inboxMenuItem:
                intent = new Intent(this, InboxActivity.class);
                break;

            case R.id.checkInMenuItem:
                intent = new Intent(this, CheckInActivity.class);
                break;

            case R.id.updatePasswordMenuItem:
                intent = new Intent(this, UpdatePasswordActivity.class);
            break;

            case R.id.updateProfileMenuItem:
                intent = new Intent(this, UpdateProfileActivity.class);
            break;

            case R.id.managePhotosMenuItem:
                intent = new Intent(this, ManagePhotosActivity.class);
            break;

            case R.id.logoutMenuItem:
                intent = new Intent(this, LogoutActivity.class);
            break;
        }

        if(null != intent) {
            startActivity(intent);
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}

package com.wingmand.mainactivityfragments;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentStatePagerAdapter;

import com.wingmand.mainactivityfragments.InYourCityFragment;
import com.wingmand.mainactivityfragments.InYourStateFragment;
import com.wingmand.mainactivityfragments.NearYouNowFragment;

public class MainActivityRootTabPagerAdapter extends FragmentStatePagerAdapter {
    int mNumOfTabs;

    public MainActivityRootTabPagerAdapter(FragmentManager fm, int NumOfTabs) {
        super(fm);
        this.mNumOfTabs = NumOfTabs;
    }

    @Override
    public Fragment getItem(int position) {

        switch (position) {
            case 0:
                NearYouNowFragment nf = new NearYouNowFragment();
                return nf;
            default:
                return null;
        }
    }

    @Override
    public int getCount() {
        return mNumOfTabs;
    }
}

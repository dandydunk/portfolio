package com.wingmand.inboxfragments;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentStatePagerAdapter;

public class InboxActivityRootTabPagerAdapter extends FragmentStatePagerAdapter {
    int mNumOfTabs;

    public InboxActivityRootTabPagerAdapter(FragmentManager fm, int NumOfTabs) {
        super(fm);
        this.mNumOfTabs = NumOfTabs;
    }

    @Override
    public Fragment getItem(int position) {

        switch (position) {
            case 0:
                MessagesFragment f = new MessagesFragment();
                return f;
            case 1:
                RequestsFragment f2 = new RequestsFragment();
                return f2;
            default:
                return null;
        }
    }

    @Override
    public int getCount() {
        return mNumOfTabs;
    }
}

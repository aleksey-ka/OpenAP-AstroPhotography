// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef FILTERWHEEL_H
#define FILTERWHEEL_H

class FilterWheel {
public:
    bool Open();
    void Close();

    int GetSlotsCount() const { return slotsCount; };

    int GetPosition();
    void SetPosition( int );

private:
    int id;
    int slotsCount = 0;
};

#endif // FILTERWHEEL_H

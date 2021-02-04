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

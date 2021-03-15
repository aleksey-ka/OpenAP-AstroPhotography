// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef FOCUSER_H
#define FOCUSER_H

class Focuser {
public:
    virtual void Close() = 0;

    virtual void Forward() = 0;
    virtual void Backward() = 0;

    virtual void StepUp() = 0;
    virtual void StepDown() = 0;
    virtual int StepsPerMove() const = 0;

    virtual void MarkZero() = 0;
    virtual void GoToPos( int ) = 0;
    virtual int GetPos() const = 0;

    virtual ~Focuser() {}
};

#endif // FOCUSER_H

#ifndef CM_POLL_COND_HH_
#define CM_POLL_COND_HH_


// // PollThread si nastavi ChangeRqTime(), cimz rekne, kdy chce vzbudit.
// //
// // Kdyz se chce uspat, tak si zavola UpdateSleepTime, aby vedel, za jak dlouho
// // se ma vzbudit.
// //
// // Az se vzbudi, tak si zavola NeedCall(), aby vedel, jestli tohle bylo
// // vzbuzeni pro nas. Pokud ano, muze si zase zavolat ChangeRqTime(), aby si
// // nastavil novy cas vzbuzeni.
// //
// // Pokud jiny thread chce ten PollThread vzbudit, tak zavola Wake.
// //
// class CCmPollCond
// {
// public:
//     CMSCondition *condition;
//     double RqTime;
//     bool CalledNow;
//     bool FirstNextTime;

//     void init(CMSCondition *_condition, double _RqTime, bool _CalledNow, bool _FirstNextTime)
//     { condition = _condition; RqTime = _RqTime; CalledNow = _CalledNow; FirstNextTime = _FirstNextTime; }
//     void Wake(void)
//     {  CalledNow=true; condition->Broadcast(); }

//     void ClearCallRq(void)
//     {  CalledNow = false; FirstNextTime = false; RqTime = 0.0; }

//     void ChangeRqTime(double _RqTime) { RqTime = _RqTime; }

//     bool NeedCall(double ActTime) const
//     {
//         double Tolerance = LIB_CM_TIME_TOLERANCE;
//         if ((CalledNow) || (FirstNextTime) || ((ActTime+Tolerance)>RqTime))
//             return true;
//         return false;
//     }

//     void UpdateSleepTime(double ActTime, double &sleep_time) const
//     {
//         double calc_sleep_time = 0.0;
//         if (RqTime > ActTime) calc_sleep_time = RqTime - ActTime;
//         if ((calc_sleep_time > 0.0) && ((sleep_time > calc_sleep_time) || (sleep_time == 0.0)))
//             sleep_time = calc_sleep_time;
//     }
// };

#endif

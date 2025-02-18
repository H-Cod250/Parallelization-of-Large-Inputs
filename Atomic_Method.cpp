_day_index with a different value and producing wrong results.
Figure 7. Atomic Version Program Execution – Block 3 Results and Execution Time
Page 20 of 38
#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <omp.h>
using namespace std;
const int days = 5;
int numEmployees;
const char *Days[days] = {"SUN", "MON", "TUE", "WED", "THU"};
vector<string> generateEmployees(int size)
{
vector<string> employees(size);
#pragma omp parallel for
for (int i = 0; i < size; i++) {
employees[i] = "employee_" + to_string(i + 1);
}
return employees;
}
int main()
{
double total_start_time = omp_get_wtime();
int total[5] = {0}, max = 0, indexMax, min, indexMin;
printf("\n\nEnter the number of employees to generate: ");
scanf("%d", &numEmployees);
vector<string> Staff = generateEmployees(numEmployees);
vector<vector<int>> workingHours(numEmployees, vector<int>(days));
srand(time(0));
#pragma omp parallel for
for (int i = 0; i < numEmployees; i++)
{
for (int j = 0; j < days; j++)
{
workingHours[i][j] = rand() % 12 + 1;
}
}
printf("\n\n***************** Block#1: Table of Employees and their Weekly Working Hours
****************** \n\n");
printf("\n\t\t\t");
for (int i = 0; i < days; i++)
{
printf("%s\t", Days[i]);
}
printf("\n");
#pragma omp parallel for ordered
for (int i = 0; i < numEmployees; i++)
{
#pragma omp ordered
{
printf("Thread %d: %s\t", omp_get_thread_num(), Staff[i].c_str());
for (int j = 0; j < days; j++)
{
printf("%d\t", workingHours[i][j]);
}
printf("\n");
}
Page 21 of 38
}
/////////////////////////////////////////////////////////////////////////////////////////////////
printf("\n\n***************** Block#2: Calculating the Highest and Lowest Working-hours Days
****************** \n\n");
printf("Displaying each day with thier total working hours:");
#pragma omp parallel for ordered
for (int j = 0; j < days; j++)
{
for (int i = 0; i < numEmployees; i++)
{
#pragma omp atomic // using atomic to safely access the total array and update its values
total[j] += workingHours[i][j];
}
#pragma omp ordered
printf("\nThread: %d\ttotal of %s: %d", omp_get_thread_num(), Days[j], total[j]);
}
int threadNumMax = 0, threadNumMin = 0;
// initializining and declaring locks for synchronizing access to shared data
omp_lock_t maxLock, minLock;
omp_init_lock(&maxLock);
omp_init_lock(&minLock);
printf("\n\nFinding the day with maximum total working hours:");
#pragma omp parallel for
for (int i = 0; i < days; i++)
{
omp_set_lock(&maxLock);
if (total[i] > max)
{
printf("\nThread: %d\tRepresents day: %s\tBefore updating max: %d", omp_get_thread_num(),
Days[i], max);
max = total[i];
indexMax = i;
threadNumMax = omp_get_thread_num();
printf("\nThread: %d\tRepresents day: %s\tUpdated max: %d", omp_get_thread_num(), Days[i],
max);
}else{
printf("\nThread: %d\tRepresents day: %s\ttotal: %d < max: %d\tCannot update max",
omp_get_thread_num(), Days[i], total[i], max);
}
omp_unset_lock(&maxLock);
}
printf("\nResults: Thread %d updated max to %d for day %s\n", threadNumMax, max,
Days[indexMax]);
printf("\nFinding the day with minimum total working hours:");
min = max;
#pragma omp parallel for
for (int i = 0; i < days; i++)
{
omp_set_lock(&minLock);
if (total[i] < min)
{
min = total[i];
indexMin = i;
threadNumMin = omp_get_thread_num();
}
omp_unset_lock(&minLock);
}
Page 22 of 38
printf("\nResults: Thread %d updated min to %d for day %s\n", threadNumMin, min,
Days[indexMin]);
omp_destroy_lock(&maxLock);
omp_destroy_lock(&minLock);
printf("\n\n~~~The day that most employees have high working hours: %s\n", Days[indexMax]);
printf("~~~The day that most employees have low working hours: %s\n", Days[indexMin]);
/////////////////////////////////////////////////////////////////////////////////////////////////
printf("\n\n***************** Block#3: Employee-specific operations ****************** \n\n");
string employee;
int emp_weeklyH = 0, highest_hours_day, day_index = 0, emp_index, extra_day_index = -1, salary
= 0, total_hours = 0, name = 0;
bool emp_exists = false;
int threadnum;
printf("Enter an employee to find information about working hours in the week, the day of high
working hour and salary: ");
cin >> employee;
printf("\n");
omp_lock_t empLock;
omp_init_lock(&empLock);
#pragma omp parallel for
for (int i = 0; i < numEmployees; i++)
{
if (employee == Staff[i])
{
omp_set_lock(&empLock);
emp_exists = true;
emp_index = i;
highest_hours_day = workingHours[i][0];
threadnum = omp_get_thread_num();
omp_unset_lock(&empLock);
}
}
omp_destroy_lock(&empLock);
if (!emp_exists)
{
printf("\n\nSorry, this employee does not exist.\n");
}
else
{
printf("\nOutside the parallel region that checks the existance of the requested employee
(%s):\nThe Thread: %d is the one who found the match\nThe returned employee index is:
%d\nemp_exists= %d\n",employee.c_str(), threadnum ,emp_index,emp_exists);
printf("\nDisplaying %s's record:\nSUN\tMON\tTUE\tWED\tTHU\n", employee.c_str());
for (int j = 0; j < days; j++)
{
printf("%d\t", workingHours[emp_index][j]);
}
printf("\n");
printf("\nCalucalting the weekly working hours of %s:", employee.c_str());
omp_lock_t highestDayLock, locky;
omp_init_lock(&highestDayLock);
omp_init_lock(&locky);
Page 23 of 38
#pragma omp parallel for
for (int j = 0; j < days; j++)
{
omp_set_lock(&locky);
printf("\nThread %d: Adding %d hours of day %s to total weekly hours\tBefore adding:
%d)",omp_get_thread_num(), workingHours[emp_index][j], Days[j], emp_weeklyH);
emp_weeklyH += workingHours[emp_index][j];
printf("\tAfter Adding: %d", emp_weeklyH);
omp_unset_lock(&locky);
if (highest_hours_day < workingHours[emp_index][j])
{
omp_set_lock(&highestDayLock);
highest_hours_day = workingHours[emp_index][j];
day_index = j;
omp_unset_lock(&highestDayLock);
}
if (highest_hours_day == workingHours[emp_index][j] && j != day_index)
{
omp_set_lock(&highestDayLock);
extra_day_index = j;
omp_unset_lock(&highestDayLock);
}
}
omp_destroy_lock(&highestDayLock);
omp_destroy_lock(&locky);
printf("\n\n%s has worked %d hours in the week.\n", employee.c_str(), emp_weeklyH);
if (extra_day_index != -1){
printf("The days that %s has high working hours are %s and %s\n", employee.c_str(),
Days[day_index], Days[extra_day_index]);
} else {
printf("The day that %s has high working hours is %s\n", employee.c_str(),
Days[day_index]);
}
#pragma omp parallel for
for (int i = 0; i < days; i++)
{
if (workingHours[emp_index][i] > 9)
{
#pragma omp atomic // using atomic to safely access the total_hours and update its value
total_hours += 9 + (2 * (workingHours[emp_index][i] - 9));
} else {
#pragma omp atomic // using atomic to safely access the total_hours and update its value
total_hours += workingHours[emp_index][i];
}
}
salary = total_hours * 100;
printf("%s Salary: %d SAR in the week.\n", employee.c_str(), salary);
}
/////////////////////////////////////////////////////////////////////////////////////////////////
printf("\n\n********************************** EXECUTION TIME STATISTICS
********************************** \n\n");
double total_end_time = omp_get_wtime(); // should be outside
printf("\n\n~~~Total time taken for the entire program execution: %.6f seconds", total_end_time
- total_start_time);
printf("\n~~~Total time taken for the entire program execution: %.6f minutes\n\n",
(total_end_time - total_start_time) / 60);
printf("\n\n****************************** END OF EXECUTION TIME STATISTICS
****************************** \n\n");
return 0;
}
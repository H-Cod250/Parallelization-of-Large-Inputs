#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <omp.h>
using namespace std;

// Global variables
const int days = 5;
int numEmployees;
const char *Days[days] = {"SUN", "MON", "TUE", "WED", "THU"};

// Function to generate employee names
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
  // Start total execution time at the beginning of all operations
  double total_start_time = omp_get_wtime();

  int total[5] = {0}, max = 0, indexMax, min, indexMin;

  // Ask the user for a number of employees to generate
  printf("\n\nEnter the number of employees to generate: ");
  scanf("%d", &numEmployees);

  // Dynamically generate employee names
  vector<string> Staff = generateEmployees(numEmployees);

  // Dynamically generate random working hours for each employee
  vector<vector<int>> workingHours(numEmployees, vector<int>(days));
  srand(time(0)); // Initialize random seed
  // NO RACE CONDITION
  #pragma omp parallel for
  for (int i = 0; i < numEmployees; i++)
  {
    for (int j = 0; j < days; j++)
    {
      workingHours[i][j] = rand() % 12 + 1; // Random hours between 1 and 12
    }
  }

  printf("\n\n***************** Block#1: Table of Employees and their Weekly Working Hours ****************** \n\n");
  // Print header for working hours
  printf("\n\t\t\t");
  for (int i = 0; i < days; i++)
  {
    printf("%s\t", Days[i]);
  }
  printf("\n");

// Print employees and their working hours safely
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
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  printf("\n\n***************** Block#2: Calculating the Highest and Lowest Working-hours Days ****************** \n\n");
  // Find the day with the most and least total working hours

  printf("Displaying each day with their total working hours:"); 

  // This loop will sum the total hours for each day of the week
  #pragma omp parallel for reduction(+:total[:days]) ordered
  for (int j = 0; j < days; j++) {
    #pragma omp ordered
  {
    for (int i = 0; i < numEmployees; i++)
    {
      total[j] += workingHours[i][j];
    }
  }
  }

// This loop will compare total values between the days and returns the day with the most and least worked hours
  int threadNumMax = 0, threadNumMin = 0;

  omp_lock_t maxLock, minLock;
  omp_init_lock(&maxLock);
  omp_init_lock(&minLock);

  // Find max --------------------------------------------------
  printf("\n\nFinding the day with maximum total working hours:");
  #pragma omp parallel for
  for (int i = 0; i < days; i++)
  {
    omp_set_lock(&maxLock);
    if (total[i] > max)
    {
      printf("\nThread: %d\tRepresents day: %s\tBefore updating max: %d", omp_get_thread_num(), Days[i], max);
      max = total[i];
      indexMax = i;
      threadNumMax = omp_get_thread_num();
      printf("\nThread: %d\tRepresents day: %s\tUpdated max: %d", omp_get_thread_num(), Days[i], max);
    }else{
      printf("\nThread: %d\tRepresents day: %s\ttotal: %d < max: %d\tCannot update max", omp_get_thread_num(), Days[i], total[i], max);
    }
    omp_unset_lock(&maxLock);
  }
  printf("\nResults: Thread %d updated max to %d for day %s\n", threadNumMax, max, Days[indexMax]);

  // Find min --------------------------------------------------
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
  printf("\nResults: Thread %d updated min to %d for day %s\n", threadNumMin, min, Days[indexMin]);

  omp_destroy_lock(&maxLock);
  omp_destroy_lock(&minLock);

  printf("\n\n~~~The day that most employees have high working hours: %s\n", Days[indexMax]);
  printf("~~~The day that most employees have low working hours: %s\n", Days[indexMin]);


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  printf("\n\n***************** Block#3: Employee-specific operations ****************** \n\n");

  string employee;
  int emp_weeklyH = 0, highest_hours_day, day_index = 0, emp_index, extra_day_index = -1, salary = 0, total_hours = 0, name = 0;
  bool emp_exists = false; int threadnum;


  printf("Enter an employee to find information about working hours in the week, the day of high working hour and salary: ");
  cin >> employee;
  printf("\n");

 // Check if the employee exists
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
        highest_hours_day = workingHours[i][0];  // Initialize with Sunday hours
        threadnum = omp_get_thread_num();
        omp_unset_lock(&empLock);
    }
}
omp_destroy_lock(&empLock);

if (!emp_exists)
{
    printf("Sorry, this employee does not exist.\n");
}
else
{
    printf("\nOutside the parallel region that checks the existance of the requested employee (%s):\nThe Thread: %d is the one who found the match\nThe returned employee index is: %d\nemp_exists= %d\n",employee.c_str(), threadnum ,emp_index,emp_exists);
       
       // printing the fecord of the requested employee:
    printf("\nDisplaying %s's record:\nSUN\tMON\tTUE\tWED\tTHU\n", employee.c_str());
    for (int j = 0; j < days; j++)
      {
        printf("%d\t", workingHours[emp_index][j]);
      }
      printf("\n");

    
    // Calculate weekly hours and find the day/s with the highest working hours
    omp_lock_t highestDayLock;
    omp_init_lock(&highestDayLock);
    
    // Lock for weekly hours accumulation
    omp_lock_t weeklyLock;
    omp_init_lock(&weeklyLock);

    printf("\nCalucalting the weekly working hours of %s:", employee.c_str());

    #pragma omp parallel for
    for (int j = 0; j < days; j++)
    {
        // Lock for weekly hours accumulation
        omp_set_lock(&weeklyLock);
        printf("\nThread %d: Adding %d hours of day %s to total weekly hours\tBefore adding: %d)",omp_get_thread_num(), workingHours[emp_index][j], Days[j], emp_weeklyH);
        emp_weeklyH += workingHours[emp_index][j];
        omp_unset_lock(&weeklyLock);
        printf("\tAfter Adding: %d", emp_weeklyH);

        // Check for the highest working hours
        if (highest_hours_day < workingHours[emp_index][j])
        {
            omp_set_lock(&highestDayLock);
            highest_hours_day = workingHours[emp_index][j];
            day_index = j; // Day with the highest worked hours
            omp_unset_lock(&highestDayLock);
        }

        // If there's a tie for the highest working hours
        if (highest_hours_day == workingHours[emp_index][j] && j != day_index)
        {
            omp_set_lock(&highestDayLock);
            extra_day_index = j; // Another day with the same working hours
            omp_unset_lock(&highestDayLock);
        }
    }
    omp_destroy_lock(&highestDayLock);
    omp_destroy_lock(&weeklyLock);

    // Output the result
    printf("\n\n%s has worked %d hours in the week.\n", employee.c_str(), emp_weeklyH);
    if (extra_day_index != -1)
    {
        printf("The days that %s has high working hours are %s and %s\n", employee.c_str(), Days[day_index], Days[extra_day_index]);
    }
    else
    {
        printf("The day that %s has high working hours is %s\n", employee.c_str(), Days[day_index]);
    }
}
// Calculate salary
#pragma omp parallel for reduction(+:total_hours)
    for (int i = 0; i < days; i++) {
        if (workingHours[emp_index][i] > 9) {
            total_hours += 9 + (2 * (workingHours[emp_index][i] - 9));  // Bonus for hours > 9
        } else {
            total_hours += workingHours[emp_index][i];  // Normal hours
        }
    }
    // Calculate salary based on total hours
    salary = total_hours * 100;  // Example rate of 100 SAR per hour
    printf("%s Salary: %d SAR in the week.\n", employee.c_str(), salary);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  printf("\n\n********************************** EXECUTION TIME STATISTICS ********************************** \n\n");

  double total_end_time = omp_get_wtime(); // should be outside
  printf("\n\n~~~Total time taken for the entire program execution: %.6f seconds", total_end_time - total_start_time);
  printf("\n~~~Total time taken for the entire program execution: %.6f minutes\n\n", (total_end_time - total_start_time) / 60);
  printf("\n\n****************************** END OF EXECUTION TIME STATISTICS ****************************** \n\n");

  return 0;
}

package com.example.android.hangboard;

import android.arch.lifecycle.LiveData;
import android.arch.persistence.room.Dao;
import android.arch.persistence.room.Delete;
import android.arch.persistence.room.Insert;
import android.arch.persistence.room.Query;
import android.arch.persistence.room.Update;

import java.util.List;

@Dao
public interface WorkoutDAO {
    @Insert
    public void insert(Workout... workouts);

    @Update
    public void update(Workout... workouts);

    @Delete
    public void delete(Workout workout);

    @Query("SELECT * FROM workout")
    public LiveData<List<Workout>> getWorkouts();

    @Query("SELECT * FROM workout WHERE workoutTitle = :title")
    public LiveData<Workout> getWorkoutWithTitle(String title);

}

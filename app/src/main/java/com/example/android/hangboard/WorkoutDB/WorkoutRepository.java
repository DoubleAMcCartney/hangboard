package com.example.android.hangboard.WorkoutDB;

import android.app.Application;
import android.arch.lifecycle.LiveData;
import android.os.AsyncTask;

import java.util.List;

public class WorkoutRepository {
    private WorkoutDAO WorkoutDAO;
    private LiveData<List<Workout>> allWorkouts;

    public WorkoutRepository(Application application) {
        WorkoutDatabase database = WorkoutDatabase.getInstance(application);
        WorkoutDAO = database.getWorkoutDAO();
        allWorkouts = WorkoutDAO.getWorkouts();
    }

    public void insert(Workout Workout) {
        new InsertWorkoutAsyncTask(WorkoutDAO).execute(Workout);
    }

    public void update(Workout Workout) {
        new UpdateWorkoutAsyncTask(WorkoutDAO).execute(Workout);
    }

    public void delete(Workout Workout) {
        new DeleteWorkoutAsyncTask(WorkoutDAO).execute(Workout);
    }

    public LiveData<List<Workout>> getAllWorkouts() {
        return allWorkouts;
    }

    private static class InsertWorkoutAsyncTask extends AsyncTask<Workout, Void, Void> {
        private WorkoutDAO WorkoutDAO;

        private InsertWorkoutAsyncTask(WorkoutDAO WorkoutDAO) {
            this.WorkoutDAO = WorkoutDAO;
        }

        @Override
        protected Void doInBackground(Workout... Workouts) {
            WorkoutDAO.insert(Workouts[0]);
            return null;
        }
    }

    private static class UpdateWorkoutAsyncTask extends AsyncTask<Workout, Void, Void> {
        private WorkoutDAO WorkoutDAO;

        private UpdateWorkoutAsyncTask(WorkoutDAO WorkoutDAO) {
            this.WorkoutDAO = WorkoutDAO;
        }

        @Override
        protected Void doInBackground(Workout... Workouts) {
            WorkoutDAO.update(Workouts[0]);
            return null;
        }
    }

    private static class DeleteWorkoutAsyncTask extends AsyncTask<Workout, Void, Void> {
        private WorkoutDAO WorkoutDAO;

        private DeleteWorkoutAsyncTask(WorkoutDAO WorkoutDAO) {
            this.WorkoutDAO = WorkoutDAO;
        }

        @Override
        protected Void doInBackground(Workout... Workouts) {
            WorkoutDAO.delete(Workouts[0]);
            return null;
        }
    }

}

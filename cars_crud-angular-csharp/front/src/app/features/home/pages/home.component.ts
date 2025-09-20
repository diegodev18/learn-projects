import { Component, OnInit } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { CommonModule } from '@angular/common';
import type { Car } from '../models/car.model';
import type { Action } from '../models/action.model';
import { CarService } from '../../../core/services/car.service';

@Component({
  selector: 'app-home',
  templateUrl: './home.component.html',
  styleUrls: ['./home.component.css'],
  standalone: true,
  imports: [FormsModule, CommonModule],
})
export class HomeComponent implements OnInit {
  title = 'Bienvenido a la página Home';
  cars: Car[] = [] as Car[];
  car: Car = {} as Car;
  error: string | null = null;
  action: Action = 'adding';
  loading = false;

  constructor(private carService: CarService) {}

  ngOnInit() {
    this.loadCars();
  }

  loadCars() {
    this.loading = true;
    this.carService.getCars().subscribe({
      next: (cars) => {
        this.cars = cars;
        this.loading = false;
      },
      error: (error) => {
        if (error.status === 0) {
          this.error =
            'Network error: Please check if the API server is running.';
        }
        this.loading = false;
      },
    });
  }

  addCar() {
    if (
      Object.values(this.car).filter((value) => value !== null && value !== '')
        .length < 3
    ) {
      return;
    }

    this.loading = true;
    const newCarData = {
      model: this.car.model,
      brand: this.car.brand,
      year: this.car.year,
    };

    this.carService.createCar(newCarData).subscribe({
      next: (newCar) => {
        this.cars.push(newCar);
        this.car = {} as Car;
        this.loading = false;
      },
      error: (error) => {
        console.error('Error creating car:', error);
        this.loading = false;
      },
    });
  }

  setEditingCar(id: number) {
    this.action = 'editing';

    const carFound = this.cars.find((car) => car.id === id);
    if (carFound) {
      this.car = { ...carFound };
    }
  }

  editCar(id: number) {
    if (
      Object.values(this.car).filter((value) => value !== null && value !== '')
        .length < 4
    ) {
      console.log(this.car);
      return;
    }

    this.loading = true;
    this.carService.updateCar(id, this.car).subscribe({
      next: () => {
        const carFoundIndex = this.cars.findIndex((car) => car.id === id);
        if (carFoundIndex !== -1) {
          this.cars[carFoundIndex] = this.car;
          this.action = 'adding';
          this.car = {} as Car;
        }
        this.loading = false;
      },
      error: (error) => {
        console.error('Error updating car:', error);
        this.loading = false;
      },
    });
  }

  removeCar(id: number, idx: number) {
    if (this.cars[idx].id !== id) {
      console.error('Index does not match the car ID');
      return;
    }

    this.loading = true;
    this.carService.deleteCar(id).subscribe({
      next: () => {
        this.cars.splice(idx, 1);
        this.loading = false;
      },
      error: (error) => {
        console.error('Error deleting car:', error);
        this.loading = false;
      },
    });
  }
}

import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable, of } from 'rxjs';
import { Car } from '../../features/home/models/car.model';

@Injectable({
  providedIn: 'root',
})
export class CarService {
  private apiUrl = 'http://localhost:5198/api'; // Placeholder API
  // You can replace this with your actual API endpoint

  constructor(private http: HttpClient) {}

  getCars(): Observable<Car[]> {
    return this.http.get<Car[]>(`${this.apiUrl}/cars`);
  }

  createCar(car: Omit<Car, 'id'>): Observable<Car> {
    const newCar: Car = {
      ...car,
      id: Math.floor(Math.random() * 1000) + 100, // Generate random ID for demo
    };
    return this.http.post<Car>(`${this.apiUrl}/cars`, newCar);
  }

  updateCar(id: number, car: Car): Observable<Car> {
    return this.http.put<Car>(`${this.apiUrl}/cars/${id}`, car);
  }

  deleteCar(id: number): Observable<void> {
    return this.http.delete<void>(`${this.apiUrl}/cars/${id}`);
  }
}

using CrudCarsApi.Models;
using System.Collections.Generic;
using System.Linq;

namespace CrudCarsApi.Services
{
    public class CarService : ICarService
    {
        private readonly List<Car> _cars = new List<Car>();

        public IEnumerable<Car> GetAll()
        {
            return _cars;
        }

        public Car? GetById(int id)
        {
            var carFound = _cars.FirstOrDefault(c => c.Id == id);

            if (carFound == null)
                return null;

            return carFound;
        }

        public Car Create(Car car)
        {
            car.Id = _cars.Count > 0 ? _cars.Max(c => c.Id) + 1 : 1;
            _cars.Add(car);
            return car;
        }

        public void Update(int id, Car car)
        {
            var existing = GetById(id);
            if (existing == null)
                return;

            existing.Model = car.Model;
            existing.Brand = car.Brand;
            existing.Year = car.Year;
        }

        public void Delete(int id)
        {
            var car = GetById(id);
            if (car == null)
                return;

            _cars.Remove(car);
        }
    }
}

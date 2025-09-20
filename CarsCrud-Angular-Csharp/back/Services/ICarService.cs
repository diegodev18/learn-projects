using CrudCarsApi.Models;
using System.Collections.Generic;

namespace CrudCarsApi.Services
{
    public interface ICarService
    {
        IEnumerable<Car> GetAll();
        Car? GetById(int id);
        Car Create(Car car);
        void Update(int id, Car car);
        void Delete(int id);
    }
}

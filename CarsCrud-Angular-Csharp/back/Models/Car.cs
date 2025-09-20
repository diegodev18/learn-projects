using System.ComponentModel.DataAnnotations;

namespace CrudCarsApi.Models
{
    public class Car
    {
        public int Id { get; set; }
        public string Model { get; set; } = string.Empty;
        public string Brand { get; set; } = string.Empty;
        public int Year { get; set; } = 0;
    }
}
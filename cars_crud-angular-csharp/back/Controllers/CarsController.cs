using CrudCarsApi.Models;
using CrudCarsApi.Services;
using Microsoft.AspNetCore.Mvc;
using System.Collections.Generic;

namespace CrudCarsApi.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class CarsController(ICarService carService) : ControllerBase
    {
        private readonly ICarService _carService = carService;

        [HttpGet]
        public ActionResult<IEnumerable<Car>> Get()
        {
            return Ok(_carService.GetAll());
        }

        [HttpPost]
        public ActionResult<Car> Post([FromBody] Car car)
        {
            var created = _carService.Create(car);
            return CreatedAtAction(nameof(Get), new { id = created.Id }, created);
        }

        [HttpPut("{id}")]
        public IActionResult Put(int id, [FromBody] Car car)
        {
            var existing = _carService.GetById(id);
            if (existing == null)
                return NotFound();

            _carService.Update(id, car);
            return NoContent();
        }

        [HttpDelete("{id}")]
        public IActionResult Delete(int id)
        {
            var existing = _carService.GetById(id);
            if (existing == null)
                return NotFound();

            _carService.Delete(id);
            return NoContent();
        }
    }
}
